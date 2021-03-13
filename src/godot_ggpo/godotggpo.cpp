#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable: 4996)
#pragma warning(disable: 4828)
#endif

#include "godotggpo.h"
#include "ggponet.h"
#include <scene\main\node.h>
#include <core\engine.h>


#define MAX_PLAYERS 4
#define MAX_PREDICTION_FRAMES 8
#define MAX_SPECTATORS 32

#define INVALID_HANDLE (-1)


// delete later
GGPOSession* GGPOPtr;
Object *GameStateRef;

//--------------

GGPO* GGPO::singleton = NULL;
int gs[10] = { 0 };

GGPO::GGPO() {
	singleton = this;
}

GGPO::~GGPO() {
	singleton = NULL;
}

GGPO *GGPO::get_singleton() {
	return GGPO::singleton;
}


void addinputs(int inputs[], Array a);

// Used to begin a new GGPO.net session. The ggpo object returned by ggpo_start_session uniquely identifies the state for this session and should be passed to all other functions.
int GGPO::startSession(const String& game, int numPlayers, int localPort) {
    GGPOSessionCallbacks cb;

    cb.begin_game = &Callbacks::begin_game;
    cb.advance_frame = &Callbacks::advance_frame;
    cb.load_game_state = &Callbacks::load_game_state;
    cb.log_game_state = &Callbacks::log_game_state;
    cb.save_game_state = &Callbacks::save_game_state;
    cb.free_buffer = &Callbacks::free_buffer;
    cb.on_event = &Callbacks::on_event;
	
    auto result = ggpo_start_session(&GGPOPtr, &cb, game.utf8().get_data(), numPlayers, sizeof(int), localPort);
    return result;
}
// Used to begin a new GGPO.net sync test session. During a sync test, every frame of execution is run twice: once in prediction mode and once again to verify the result of the prediction. If the checksums of your save states do not match, the test is aborted.
int GGPO::startSynctest(const String& game, int numPlayers, int frames) {
    GGPOSessionCallbacks cb;

    cb.begin_game = &Callbacks::begin_game;
    cb.advance_frame = &Callbacks::advance_frame;
    cb.load_game_state = &Callbacks::load_game_state;
    cb.log_game_state = &Callbacks::log_game_state;
    cb.save_game_state = &Callbacks::save_game_state;
    cb.free_buffer = &Callbacks::free_buffer;
    cb.on_event = &Callbacks::on_event;

    char gameTab[128];
    strcpy(gameTab, game.utf8().get_data());
	auto result = ggpo_start_synctest(&GGPOPtr, &cb, gameTab, numPlayers, sizeof(uint64_t), frames);
    return result;
}


// Start a spectator session.
int GGPO::startSpectating(const String& game, int numPlayers, int localPort, const String& hostIp, int hostPort) {
    GGPOSessionCallbacks cb;

    cb.begin_game = &Callbacks::begin_game;
    cb.advance_frame = &Callbacks::advance_frame;
    cb.load_game_state = &Callbacks::load_game_state;
    cb.log_game_state = &Callbacks::log_game_state;
    cb.save_game_state = &Callbacks::save_game_state;
    cb.free_buffer = &Callbacks::free_buffer;
    cb.on_event = &Callbacks::on_event;

    char hostIpTab[16];
    strcpy(hostIpTab, hostIp.utf8().get_data());
	auto result = ggpo_start_spectating(&GGPOPtr, &cb, game.utf8().get_data(), numPlayers, sizeof(uint64_t), localPort, hostIpTab, hostPort);
    return result;
}
// The time to wait before the first GGPO_EVENTCODE_CONNECTION_INTERRUPTED timeout will be sent.
int GGPO::setDisconnectNotifyStart(int timeout) {
	return ggpo_set_disconnect_notify_start(GGPOPtr, timeout);
}
// Sets the disconnect timeout. The session will automatically disconnect from a remote peer if it has not received a packet in the timeout window. You will be notified of the disconnect via a GGPO_EVENTCODE_DISCONNECTED_FROM_PEER event. NOTE: Setting a timeout value of 0 will disable automatic disconnects.
int GGPO::setDisconnectTimeout(int timeout) {
	return ggpo_set_disconnect_timeout(GGPOPtr, timeout);
}
// You should call ggpo_synchronize_input before every frame of execution, including those frames which happen during rollback.
Dictionary GGPO::synchronizeInput(int length) {
	int inputs[MAX_PLAYERS] = { 0 };
	int disconnectFlags = 0;
	Dictionary d;
	Array a;
	auto result = ggpo_synchronize_input(GGPOPtr, (void *)inputs, sizeof(int) * MAX_PLAYERS, &disconnectFlags);

	addinputs(inputs,a);

	d["result"] = result;
	if (result == ERRORCODE_SUCCESS) {
		d["disconnectFlags"] = disconnectFlags;
		d["inputs"] = a;
	}
    
    return d;
}
// Used to notify GGPO.net of inputs that should be transmitted to remote players. ggpo_add_local_input must be called once every frame for all player of type GGPO_PLAYERTYPE_LOCAL.
int GGPO::addLocalInput(int playerHandle, int linput) {
	int input = linput;
	return ggpo_add_local_input(GGPOPtr, playerHandle, &input, sizeof(input));
}
// Used to close a session. You must call ggpo_close_session to free the resources allocated in ggpo_start_session.
int GGPO::closeSession() {
	return ggpo_close_session(GGPOPtr);
}
// Should be called periodically by your application to give GGPO.net a chance to do some work. Most packet transmissions and rollbacks occur in ggpo_idle.
int GGPO::idle(int timeout) {
	return ggpo_idle(GGPOPtr, timeout);
}
// Must be called for each player in the session (e.g. in a 3 player session, must be called 3 times).
Dictionary GGPO::addPlayer(int playerType, int playerNum, const String& playerIpAddress, int playerPort) {
    GGPOPlayer player;
    Dictionary d;

    player.size = sizeof(GGPOPlayer);
    player.type = (GGPOPlayerType)playerType;
    player.player_num = playerNum;
    strcpy(player.u.remote.ip_address, playerIpAddress.utf8().get_data());
    player.u.remote.port = playerPort;

    int playerHandle = 0;
	auto result = ggpo_add_player(GGPOPtr, &player, &playerHandle);
    d["result"] = result;
    if(result == ERRORCODE_SUCCESS) {
        d["playerHandle"] = playerHandle;
        d["type"] = player.type;
        d["playerNum"] = player.player_num;
        d["ipAddress"] = player.u.remote.ip_address;
        d["port"] = player.u.remote.port;
    }
    
    return d;
}
// Disconnects a remote player from a game. Will return GGPO_ERRORCODE_PLAYER_DISCONNECTED if you try to disconnect a player who has already been disconnected.
int GGPO::disconnectPlayer(int playerHandle) {
	return ggpo_disconnect_player(GGPOPtr, playerHandle);
}
// Change the amount of frames ggpo will delay local input. Must be called before the first call to ggpo_synchronize_input.
int GGPO::setFrameDelay(int playerHandle, int frameDelay) {
	return ggpo_set_frame_delay(GGPOPtr, playerHandle, frameDelay);
}
// You should call ggpo_advance_frame to notify GGPO.net that you have advanced your gamestate by a single frame. You should call this everytime you advance the gamestate by a frame, event during rollbacks. GGPO.net may call your save_game_state callback before this function returns.
int GGPO::advanceFrame() {
	return ggpo_advance_frame(GGPOPtr);
}
// Used to write to the ggpo.net log.
void GGPO::log(const String& text) {
	ggpo_log(GGPOPtr, text.utf8().get_data());
}


void GGPO::ProcessRef(Object *customScriptInstance) {

	GameStateRef = customScriptInstance;
}	

void addinputs(int inputs[], Array a) {
	for (int i = 0; i < 2; i++) {
		a.append(inputs[i]);
	}
}

// Used to fetch some statistics about the quality of the network connection.
Dictionary GGPO::getNetworkStats(int playerHandle) {
    GGPONetworkStats stats;
    Dictionary d;

    auto result = ggpo_get_network_stats(GGPOPtr, playerHandle, &stats);
    d["result"] = result;
    if(result == ERRORCODE_SUCCESS) {
        d["sendQueueLen"] = stats.network.send_queue_len;
        d["recvQueueLen"] = stats.network.send_queue_len;
        d["ping"] = stats.network.ping;
        d["kbpsSent"] = stats.network.kbps_sent;
        d["localFramesBehind"] = stats.timesync.local_frames_behind;
        d["remoteFramesBehind"] = stats.timesync.remote_frames_behind;
    }

    return d;
}


int fletcher32_checksum(short* data, size_t len) {
    int sum1 = 0xffff, sum2 = 0xffff;
    while(len) {
        size_t tlen = len > 360 ? 360 : len;
        len -= tlen;
        do {
            sum1 += *data++;
            sum2 += sum1;
        } while(--tlen);
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return sum2 << 16 | sum1;
}

bool Callbacks::begin_game(const char* game) {
    return true;
}

bool Callbacks::advance_frame(int flags) {
	
	
	int inputs[MAX_PLAYERS] = { 0 };
	int disconnectFlags = 0;
	Dictionary d;
	Array a;

	ggpo_synchronize_input(GGPOPtr, (void *)inputs, sizeof(int) * MAX_PLAYERS, &disconnectFlags);
	addinputs(inputs,a);

	GGPO::get_singleton()->emit_signal("advance_frame", a);

    return true;
}

bool Callbacks::load_game_state(unsigned char* buffer, int len) {
	memcpy(&gs, buffer, len);
	Array a;
	for (int i : gs) {
		a.append(i);
	}

	GGPO::get_singleton()->emit_signal("load_game_state", a);
	
    return true;
}

bool Callbacks::log_game_state(char* filename, unsigned char* buffer, int len) {
    GGPO::get_singleton()->emit_signal("log_game_state", filename, buffer);
    return true;
}

bool Callbacks::save_game_state(unsigned char **buffer, int *len, int *checksum, int frame) {
	Array bytearray = GameStateRef->call("Save_GameState");

	for (int i = 0; i < bytearray.size(); i++) {
		gs[i] = bytearray[i];
	}

	*len = sizeof(gs);
    *buffer = (unsigned char*)malloc(*len);
    if(!*buffer)
        return false;
    
    GGPO::get_singleton()->emit_signal("save_game_state");

	memcpy(*buffer, &gs, *len);
    *checksum = fletcher32_checksum((short*)*buffer, *len / 2);

    return true;
}

void Callbacks::free_buffer(void* buffer) {
    free(buffer);
}

bool Callbacks::on_event(GGPOEvent* info) {
    int player, count, total, frames_ahead, disconnect_timeout;
    switch(info->code) {
        case GGPO::EVENTCODE_CONNECTED_TO_PEER: {
            player = info->u.connected.player;
            GGPO::get_singleton()->emit_signal("event_connected_to_peer", player);
            break;
        }
        case GGPO::EVENTCODE_SYNCHRONIZING_WITH_PEER: {
            player = info->u.synchronizing.player;
            count = info->u.synchronizing.count;
            total = info->u.synchronizing.total;
            GGPO::get_singleton()->emit_signal("event_synchronizing_with_peer", player, count, total);
            break;
        }
        case GGPO::EVENTCODE_SYNCHRONIZED_WITH_PEER: {
            player = info->u.synchronized.player;
            GGPO::get_singleton()->emit_signal("event_synchronized_with_peer", player);
            break;
        }
        case GGPO::EVENTCODE_RUNNING: {
            GGPO::get_singleton()->emit_signal("event_running");
            break;
        }
        case GGPO::EVENTCODE_DISCONNECTED_FROM_PEER: {
            player = info->u.disconnected.player;
            GGPO::get_singleton()->emit_signal("event_disconnected_from_peer", player);
            break;
        }
        case GGPO::EVENTCODE_TIMESYNC: {
            frames_ahead = info->u.timesync.frames_ahead;
            GGPO::get_singleton()->emit_signal("event_timesync", frames_ahead);
            break;
        }
        case GGPO::EVENTCODE_CONNECTION_INTERRUPTED: {
            player = info->u.connection_interrupted.player;
            disconnect_timeout = info->u.connection_interrupted.disconnect_timeout;
            GGPO::get_singleton()->emit_signal("event_connection_interrupted", player, disconnect_timeout);
            break;
        }
        case GGPO::EVENTCODE_CONNECTION_RESUMED: {
            player = info->u.connection_resumed.player;
            GGPO::get_singleton()->emit_signal("event_connection_resumed", player);
            break;
        }
    }

    return true;
}

void GGPO::_bind_methods() {
    ClassDB::bind_method(D_METHOD("startSession", "game", "numPlayers", "localPort"), &GGPO::startSession);
    ClassDB::bind_method(D_METHOD("startSynctest", "game", "numPlayers", "frames"), &GGPO::startSynctest);
    ClassDB::bind_method(D_METHOD("startSpectating", "game", "numPlayers", "localPort", "hostIp", "hostPort"), &GGPO::startSpectating);
    ClassDB::bind_method(D_METHOD("setDisconnectNotifyStart", "timeout"), &GGPO::setDisconnectNotifyStart);
    ClassDB::bind_method(D_METHOD("setDisconnectTimeout", "timeout"), &GGPO::setDisconnectTimeout);
    ClassDB::bind_method(D_METHOD("synchronizeInput", "length"), &GGPO::synchronizeInput);
    ClassDB::bind_method(D_METHOD("addLocalInput", "localPlayerHandle", "input"), &GGPO::addLocalInput);
    ClassDB::bind_method(D_METHOD("closeSession"), &GGPO::closeSession);
    ClassDB::bind_method(D_METHOD("idle", "timeout"), &GGPO::idle);
    ClassDB::bind_method(D_METHOD("addPlayer", "playerType", "playerNum", "playerIpAddress", "playerPort"), &GGPO::addPlayer, DEFVAL(""), DEFVAL(0));
    ClassDB::bind_method(D_METHOD("disconnectPlayer", "playerHandle"), &GGPO::disconnectPlayer);
    ClassDB::bind_method(D_METHOD("setFrameDelay", "playerHandle", "frameDelay"), &GGPO::setFrameDelay);
    ClassDB::bind_method(D_METHOD("advanceFrame"), &GGPO::advanceFrame);
    ClassDB::bind_method(D_METHOD("log", "text"), &GGPO::log);
    ClassDB::bind_method(D_METHOD("getNetworkStats", "playerHandle"), &GGPO::getNetworkStats);
	ClassDB::bind_method(D_METHOD("ProcessRef", "Instance"), &GGPO::ProcessRef);

    ADD_SIGNAL(MethodInfo("advance_frame", PropertyInfo(Variant::POOL_BYTE_ARRAY, "buffer")));
    ADD_SIGNAL(MethodInfo("load_game_state", PropertyInfo(Variant::POOL_BYTE_ARRAY, "buffer")));
    ADD_SIGNAL(MethodInfo("log_game_state", PropertyInfo(Variant::STRING, "filename"), PropertyInfo(Variant::POOL_BYTE_ARRAY, "buffer")));
    ADD_SIGNAL(MethodInfo("save_game_state"));
    ADD_SIGNAL(MethodInfo("event_connected_to_peer", PropertyInfo(Variant::INT, "player")));
    ADD_SIGNAL(MethodInfo("event_synchronizing_with_peer", PropertyInfo(Variant::INT, "player"), PropertyInfo(Variant::INT, "count"), PropertyInfo(Variant::INT, "total")));
    ADD_SIGNAL(MethodInfo("event_synchronized_with_peer", PropertyInfo(Variant::INT, "player")));
    ADD_SIGNAL(MethodInfo("event_running"));
    ADD_SIGNAL(MethodInfo("event_disconnected_from_peer", PropertyInfo(Variant::INT, "player")));
    ADD_SIGNAL(MethodInfo("event_timesync", PropertyInfo(Variant::INT, "frames_ahead")));
    ADD_SIGNAL(MethodInfo("event_connection_interrupted", PropertyInfo(Variant::INT, "player"), PropertyInfo(Variant::INT, "disconnect_timeout")));
    ADD_SIGNAL(MethodInfo("event_connection_resumed", PropertyInfo(Variant::INT, "player")));

    BIND_CONSTANT(PLAYERTYPE_LOCAL);
    BIND_CONSTANT(PLAYERTYPE_REMOTE);
    BIND_CONSTANT(PLAYERTYPE_SPECTATOR);
    BIND_CONSTANT(MAX_PLAYERS);
    BIND_CONSTANT(MAX_PREDICTION_FRAMES);
    BIND_CONSTANT(MAX_SPECTATORS);
    BIND_CONSTANT(INVALID_HANDLE);
    BIND_CONSTANT(ERRORCODE_SUCCESS);
    BIND_CONSTANT(ERRORCODE_GENERAL_FAILURE);
    BIND_CONSTANT(ERRORCODE_INVALID_SESSION);
    BIND_CONSTANT(ERRORCODE_INVALID_PLAYER_HANDLE);
    BIND_CONSTANT(ERRORCODE_PLAYER_OUT_OF_RANGE);
    BIND_CONSTANT(ERRORCODE_PREDICTION_THRESHOLD);
    BIND_CONSTANT(ERRORCODE_UNSUPPORTED);
    BIND_CONSTANT(ERRORCODE_NOT_SYNCHRONIZED);
    BIND_CONSTANT(ERRORCODE_IN_ROLLBACK);
    BIND_CONSTANT(ERRORCODE_INPUT_DROPPED);
    BIND_CONSTANT(ERRORCODE_PLAYER_DISCONNECTED);
    BIND_CONSTANT(ERRORCODE_TOO_MANY_SPECTATORS);
    BIND_CONSTANT(ERRORCODE_INVALID_REQUEST);
    BIND_CONSTANT(EVENTCODE_CONNECTED_TO_PEER);
    BIND_CONSTANT(EVENTCODE_SYNCHRONIZING_WITH_PEER);
    BIND_CONSTANT(EVENTCODE_SYNCHRONIZED_WITH_PEER);
    BIND_CONSTANT(EVENTCODE_RUNNING);
    BIND_CONSTANT(EVENTCODE_DISCONNECTED_FROM_PEER);
    BIND_CONSTANT(EVENTCODE_TIMESYNC);
    BIND_CONSTANT(EVENTCODE_CONNECTION_INTERRUPTED);
    BIND_CONSTANT(EVENTCODE_CONNECTION_RESUMED);
}
