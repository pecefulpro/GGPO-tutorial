extends Node

var players = []

const WALK_SPEED = 200

var number_players
var position


func _init(Number_players):
	number_players = Number_players
	
	
func Update(inputs):
	var i = 0
	for p in players:
		p.MovePlayer(inputs[i])
		i += 1

func Save_GameState():
	var GameStateArry = []
	for i in players:
		GameStateArry.append(round(i.velocity.x))
		GameStateArry.append(round(i.velocity.y))
		GameStateArry.append(round(i.position.x))
		GameStateArry.append(round(i.position.y))
	#print(str(GameStateArry) + "save")
	return GameStateArry

func Load_GameState(buffer):
	#print(str(buffer) + " loaded gamestate")
	
	#TODO: Making loading clearner 
	var i = 0
	for p in players:
		p.velocity.x = buffer[0 + i]
		p.velocity.y = buffer[1 + i]
		p.position.x = buffer[2 + i]
		p.position.y = buffer[3 + i]
		i += 4

