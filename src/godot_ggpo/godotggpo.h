#ifndef GODOTGGPO_H
#define GODOTGGPO_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable: 4996)
#pragma warning(disable: 4828)
#endif

#include <inttypes.h>

#include "ggponet.h"

#include "core/object.h"
#include "core/reference.h"
#include "core/dictionary.h"
#include "core/engine.h"

#include "callbacks.h"

class GGPO: public Object {
    GDCLASS(GGPO, Object)

public:
    enum PlayerType {
        PLAYERTYPE_LOCAL, PLAYERTYPE_REMOTE, PLAYERTYPE_SPECTATOR
    };
    enum ErrorCode {
        ERRORCODE_GENERAL_FAILURE = -1, ERRORCODE_SUCCESS = 0, ERRORCODE_INVALID_SESSION = 1, ERRORCODE_INVALID_PLAYER_HANDLE = 2, ERRORCODE_PLAYER_OUT_OF_RANGE = 3, ERRORCODE_PREDICTION_THRESHOLD = 4, ERRORCODE_UNSUPPORTED = 5, ERRORCODE_NOT_SYNCHRONIZED = 6, ERRORCODE_IN_ROLLBACK = 7, ERRORCODE_INPUT_DROPPED = 8, ERRORCODE_PLAYER_DISCONNECTED = 9, ERRORCODE_TOO_MANY_SPECTATORS = 10, ERRORCODE_INVALID_REQUEST = 11
    };
    enum EventCode {
        EVENTCODE_CONNECTED_TO_PEER = 1000, EVENTCODE_SYNCHRONIZING_WITH_PEER = 1001, EVENTCODE_SYNCHRONIZED_WITH_PEER = 1002, EVENTCODE_RUNNING = 1003, EVENTCODE_DISCONNECTED_FROM_PEER = 1004, EVENTCODE_TIMESYNC = 1005, EVENTCODE_CONNECTION_INTERRUPTED = 1006, EVENTCODE_CONNECTION_RESUMED = 1007
    };
    static GGPO* get_singleton();
    GGPO();
    ~GGPO();

    int startSession(const String& game, int numPlayers, int localPort);
    int startSynctest(const String& game, int numPlayers, int frames);
    int startSpectating(const String& game, int numPlayers, int localPort, const String& hostIp, int hostPort);
    int setDisconnectNotifyStart(int timeout);
    int setDisconnectTimeout(int timeout);
    Dictionary synchronizeInput(int length);
    int addLocalInput(int playerHandle, int input);
    int closeSession();
    int idle(int timeout);
    Dictionary addPlayer(int playerType, int playerNum, const String& playerIpAddress, int playerPort);
    int disconnectPlayer(int playerHandle);
    int setFrameDelay(int playerHandle, int frameDelay);
    int advanceFrame();
    void log(const String& text);
	void createInstance(Object *customScriptInstance, String text);
    Dictionary getNetworkStats(int playerHandle);


protected:
    static void _bind_methods();
    static GGPO* singleton;

};

#endif
