#ifndef _DccMQTT_h_
#define _DccMQTT_h_

#if __has_include("config.h")
#include "config.h"
#else
#warning config.h not found. Using defaults from config.example.h
#include "config.example.h"
#endif
#include "defines.h"

#include <PubSubClient.h>
#include <DCCEXParser.h>
#include <Queue.h>
#include <Ethernet.h>
#include <Dns.h>
#include <ObjectPool.h>
#include <limits.h>

#define MAXPAYLOAD 64
#define MAXDOMAINLENGTH 32

#define MAXTBUF 50  //!< max length of the buffer for building the topic name ;to be checked
#define MAXTMSG 120 //!< max length of the messages for a topic               ;to be checked PROGMEM ?
#define MAXTSTR 30  //!< max length of a topic string
#define MAXCONNECTID 40             // Broker connection id length incl possible prefixes
#define CLIENTIDSIZE 6              //  
#define MAXRECONNECT 5              // reconnection tries before final failure
#define MAXMQTTCONNECTIONS 20       // maximum number of unique tpoics available for subscribers
#define MAXTOPICLENGTH  20

// Define Broker configurations; Values are provided in the following order
// MQTT_BROKER_PORT 9883
// MQTT_BROKER_DOMAIN "dcclms.modelrailroad.ovh"
// MQTT_BROKER_ADDRESS 51, 210, 151, 143
// MQTT_BROKER_USER "dcccs"
// MQTT_BROKER_PASSWD "dcccs$3020"
// MQTT_BROKER_CLIENTID_PREFIX "dcc$lms-"
struct MQTTBroker
{
    int port;
    IPAddress ip;
    const FSH *domain = nullptr;
    const FSH *user = nullptr;
    const FSH *pwd = nullptr;
    const FSH *prefix = nullptr;
    byte cType; // connection type to identify valid params

    IPAddress resovleBroker(const FSH *d){
        DNSClient dns;
        IPAddress bip;

        char domain[MAXDOMAINLENGTH];      
        strcpy_P(domain, (const char *)d);

        dns.begin(Ethernet.dnsServerIP());
        if (dns.getHostByName(domain, bip) == 1)
        {
            DIAG(F("MQTT Broker/ %s = %d.%d.%d.%d"), domain, bip[0], bip[1],bip[2],bip[3]);
        }
        else
        {
            DIAG(F("MQTT Dns lookup for %s failed"), domain);
        }
        return bip;
    }

    MQTTBroker(int p, IPAddress i, const FSH *d) : port(p), ip(i), domain(d), cType(1) {};
    MQTTBroker(int p, IPAddress i, const FSH *d, const FSH *uid, const FSH *pass) : port(p), ip(i), domain(d), user(uid), pwd(pass), cType(2){};
    MQTTBroker(int p, IPAddress i, const FSH *d, const FSH *uid, const FSH *pass, const FSH *pfix) : port(p), ip(i), domain(d), user(uid), pwd(pass), prefix(pfix), cType(3){};
    MQTTBroker(int p, const FSH *d, const FSH *uid, const FSH *pass, const FSH *pfix) : port(p), domain(d), user(uid), pwd(pass), prefix(pfix), cType(4)
    {
        ip = resovleBroker(d);
    };
    MQTTBroker(int p, const FSH *d, const FSH *uid, const FSH *pass) : port(p), domain(d), user(uid), pwd(pass), cType(5)
    {
        ip = resovleBroker(d);
    };
    MQTTBroker(int p, const FSH *d) : port(p), domain(d), cType(6)
    {
        ip = resovleBroker(d);
    };
};

/**
 * @brief dcc-ex command as recieved via MQ
 * 
 */
typedef struct csmsg_t { 
	char cmd[MAXPAYLOAD];     // recieved command message
    byte mqsocket;            // from which mqsocket / subscriberid
} csmsg_t; 

typedef struct csmqttclient_t {
    int     distant;    // random int number recieved from the subscriber
    byte    mqsocket;   // mqtt socket = subscriberid provided by the cs
    int32_t topic;      // cantor(subscriber,cs) encoded tpoic used to send / recieve commands
    bool    open;       // true as soon as we have send the id to the mq broker for the client to pickup
} csmqttclient_t;

enum DccMQTTState
{
    INIT,
    CONFIGURED, // server/client objects set
    CONNECTED   // mqtt broker is connected
};

class DccMQTT
{
private:
// Methods    
        DccMQTT() = default;
        DccMQTT(const DccMQTT &);                                       // non construction-copyable
        DccMQTT &operator=(const DccMQTT &);                            // non copyable

void    setup(const FSH *id, MQTTBroker *broker);
void    connect();                                                      // (re)connects to the broker 
// Members
static  DccMQTT                         singleton;                       
        EthernetClient                  ethClient;                      // TCP Client object for the MQ Connection
        IPAddress                       server;                         // MQTT server object
        PubSubClient                    mqttClient;                     // PubSub Endpoint for data exchange
        MQTTBroker                     *broker;                         // Broker configuration object as set in config.h
 
        ObjectPool<csmsg_t,MAXPOOLSIZE> pool;                           // Pool of commands recieved for the CS
        Queue<int>                      in;                             // Queue of indexes into the pool according to incomming cmds

        char                            clientID[(CLIENTIDSIZE*2)+1];   // unique ID of the commandstation; not to confused with the connectionID
        csmqttclient_t                  clients[MAXMQTTCONNECTIONS];    // array of connected mqtt clients
        char                            connectID[MAXCONNECTID];        // clientId plus possible prefix if required by the broker
        uint8_t                         subscriberid = 0;               // id assigned to a mqtt client when recieving the inital handshake; +1 at each connection
                                    
        DccMQTTState                    mqState = INIT;
        RingStream                     *outboundRing;
        char                            buffer[MAXTMSG];                // temp buffer for manipulating strings / messages 

public:
    static DccMQTT *get() noexcept
    {
        return &singleton;
    }

    boolean subscribe(char *topic);
    void publish(char *topic, char* payload); 

    bool isConfigured() { return mqState == CONFIGURED; };
    bool isConnected() { return mqState == CONNECTED; };
    void setState(DccMQTTState s) { mqState = s; };

    ObjectPool<csmsg_t,MAXPOOLSIZE> *getPool() { return &pool; };
    Queue<int> *getIncomming() { return &in; };

    char *getClientID() { return clientID; };
    uint8_t getClientSize() { return subscriberid; }

    // initalized to 0 so that the first id comming back is 1
    // index 0 in the clients array is not used therefore
    //! improvement here to be done to save some bytes
    uint8_t obtainSubscriberID(){
        if ( subscriberid == MAXMQTTCONNECTIONS) {
            return 0;  // no more subscriber id available 
        } 
        return (++subscriberid);
    }

    // this could be calculated once forever at each new connect and be stored
    // but to save space we calculate it at each publish

    void getSubscriberTopic( uint8_t subscriberid, char *tbuffer ){
        sprintf(tbuffer, "%s/%ld/cmd", clientID, (long) clients[subscriberid].topic);
    }
    
    csmqttclient_t *getClients() { return clients; };

    void setup(); // called at setup in the main ino file
    void loop();

    ~DccMQTT() = default;
};

// /**
//  * @brief MQTT broker configuration done in config.h
//  */

// // Class for setting up the MQTT connection / topics / queues for processing commands and sendig back results

// #define MAXDEVICEID 20                  // maximum length of the unique id / device id
// #define MAXTOPICS   8                   // command L,T,S,A plus response plus admin for inital handshake
// #define TCMDROOT    "command/"          // root of command topics
// #define TCMRESROOT  "result/"           // root of the result topic
// #define ADMROOT     "admin/"            // root of the admin topic where whe can do hanshakes for the inital setup
// ;                                       // esp for sec reasons i.e. making sure we are talking to the right device and
// ;                                       // not some one elses
// #define TELEMETRYROOT "telemetry/"      // telemetry topic
// #define DIAGROOT      "diag/"           // diagnostics
// #define JRMIROOT      "jrmi/"

// #define NOOFDCCTOPICS 11
// enum DccTopics {
//     CMD_L,                               // L is Loco or Layout(power on/off)
//     CMD_T,
//     CMD_S,
//     CMD_A,
//     RESULT,
//     ADMIN,
//     TELEMETRY,
//     DIAGNOSTIC,
//     JRMI,
//     INVALID_T
// };

// /**
//  * @brief List of keywords used in the command protocol
//  *
//  */
// #define MAX_KEYWORD_LENGTH 11
// PROGMEM const char _kRead[] = {"read"};
// PROGMEM const char _kWrite[] = {"write"};
// PROGMEM const char _kPower[] = {"power"};
// PROGMEM const char _kThrottle[] = {"throttle"};
// PROGMEM const char _kFunction[] = {"function"};
// PROGMEM const char _kCv[] = {"cv"};
// PROGMEM const char _kSpeed[] = {"speed"};
// PROGMEM const char _kLocomotive[] = {"locomotive"};
// PROGMEM const char _kValue[] = {"value"};
// PROGMEM const char _kDirection[] = {"direction"};
// PROGMEM const char _kState[] = {"state"};
// PROGMEM const char _kFn[] = {"fn"};
// PROGMEM const char _kTrack[] = {"track"};
// PROGMEM const char _kBit[] = {"bit"};

// /**
//  * @brief The ingoin and outgoing queues can hold 20 messages each; this should be bigger than the number
//  * of statically allocated pool items whose pointers are getting pushed into the queues.
//  *
//  */
// #define MAXQUEUE 20     // MAX message queue length

// class DccMQTT
// {
// private:

//     static char *deviceID;              // Unique Device Identifier; based on the chip
//     static Queue<int> inComming;             // incomming messages queue; the queue only contains indexes to the message pool
//     static Queue<int> outGoing;              // outgoing messages queue; the queue only contains indexes to the message pool

// public:
//     static char **topics;               // list of pub/sub topics
//     static PubSubClient *mqClient;

//     static void setup(DCCEXParser p);   // main entry to get things going
//     static void loop();                 // recieveing commands / processing commands / publish results
//     static bool connected();            // true if the MQ client is connected

//     static char *getDeviceID();
//     static void setDeviceID();
//     static void subscribe();            // subscribes to all relevant topics
//     static void subscribeT(char *topic);// subscribe to a particular topic for other than the std ones in subscribe (e.g. telemetry)
//     static void publish();              // publishes a JSON message constructed from the outgoing queue (cid and result)
//     static void printTopics();          // prints the list of subscribed topics - debug use
//     static bool inIsEmpty();            // test if the incomming queue is empty
//     static bool outIsEmpty();           // test if the outgoing queue is empty
//     static void pushIn(uint8_t midx);   // push a command struct into the incomming queue for processing
//     static void pushOut(uint8_t midx);  // push a command struct into the incomming queue for processing
//     static uint8_t popOut();            // pop a command struct with the result to be published
//     static uint8_t popIn();             // pop a command struct from the in comming queue for processing

//     static void pub_free_memory(int fm);

//     DccMQTT();
//     ~DccMQTT();
// };

#endif