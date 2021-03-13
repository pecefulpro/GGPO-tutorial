extends Node2D


const LOCAL = 7001
const PLAYER_PORT = 7000
const PLAYER_NUMBERS = 2
const OTHER_HANDEL = 2
const PLAYER_HANDEL = 1
const IP_ADDRESS = "127.0.0.1"
const MAX_PLAYERS = 2 


var localPlayerHandle
var result
var player_instance
var gs

func _ready():
	
	Connect_Events()
	#result = GGPO.startSynctest("Test", PLAYER_NUMBERS, 7)
	result = GGPO.startSession("ark", PLAYER_NUMBERS , LOCAL)
	var localHandle = GGPO.addPlayer(GGPO.PLAYERTYPE_LOCAL,PLAYER_HANDEL,IP_ADDRESS,LOCAL)
	localPlayerHandle = localHandle["playerHandle"]
	GGPO.setFrameDelay(localPlayerHandle, 2)
	GGPO.addPlayer(GGPO.PLAYERTYPE_REMOTE ,OTHER_HANDEL,IP_ADDRESS,PLAYER_PORT)
	gs = load("res://GameState.gd").new(PLAYER_NUMBERS)
	GGPO.ProcessRef(gs)
	GGPO.addPlayer(GGPO.PLAYERTYPE_REMOTE ,OTHER_HANDEL,IP_ADDRESS,PLAYER_PORT)
	add_player(PLAYER_HANDEL)

	
func _physics_process(delta):
	GGPO_idle()
	var input = read_input()
	var result = GGPO.ERRORCODE_SUCCESS
	if localPlayerHandle != GGPO.INVALID_HANDLE:
		result = GGPO.addLocalInput(localPlayerHandle,input)
	if result == GGPO.ERRORCODE_SUCCESS:
		result = GGPO.synchronizeInput(MAX_PLAYERS)
		if result["result"] == GGPO.ERRORCODE_SUCCESS:
			Advanced_Frame(result["inputs"])
			
	

func Advanced_Frame(inputs):
	gs.Update(inputs)
	GGPO.advanceFrame()
	

func Connect_Events():
	GGPO.connect("advance_frame", self, "_onAdvanceFrame")
	GGPO.connect("load_game_state", self, "_onLoadGameState")
	GGPO.connect("log_game_state", self, "_onLogGameState")
	GGPO.connect("save_game_state", self, "_onSaveGameState")
	GGPO.connect("event_connected_to_peer", self, "_onEventConnectedToPeer")
	#GGPO.connect("event_synchronizing_with_peer", self, "_onEventSynchronizingWithPeer")
	#GGPO.connect("event_synchronized_with_peer", self, "_onEventSynchronizedWithPeer")
	#GGPO.connect("event_running", self, "_onEventRunning")
	GGPO.connect("event_disconnected_from_peer", self, "_onEventDisconnectedFromPeer")
	#GGPO.connect("event_timesync", self, "_onEventTimesync")
	#GGPO.connect("event_connection_interrupted", self, "_onEventConnectionInterrupted")
	#GGPO.connect("event_connection_resumed", self, "_onEventConnectionResumed")

func read_input():
	if Input.is_action_pressed("ui_right"):
		return 1 
	elif Input.is_action_pressed("ui_left"):
		return 2
	elif Input.is_action_pressed("ui_up"):
		return 3
	elif Input.is_action_pressed("ui_down"):
		return 4
	else:
		return 0
		
func _onSaveGameState():
	pass
	#print("On SaveGameState")

func add_player(player_handel):
	player_instance = load("res://Player.tscn").instance()
	player_instance.set_name(str(player_handel))
	player_instance.set_id_name(str(player_handel))
	get_node("/root/Root").add_child(player_instance)
	gs.players.append(player_instance)
	
	
func GGPO_idle():
	#Not necessary to do
	var start = 0
	var next = 0
	var now = 0 
	
	GGPO.idle(max(0, next - now - 1))
	
	if (now >= next):
		next = now + (1000 / 60);

func _onLoadGameState(buffer):
	gs.Load_GameState(buffer)


func _onAdvanceFrame(inputs):
	gs.Update(inputs)
	GGPO.advanceFrame()
	
func _onEventConnectedToPeer(Player):
	add_player(Player)

func _onEventDisconnectedFromPeer(Player):
	#remove_player(Player) #TODO
	GGPO.closeSession()
