extends KinematicBody2D


const GRAVITY = 200.0
const WALK_SPEED = 80
const RIGHT = 1
const LEFT = 2
const UP = 3
const DOWN = 4


var velocity = Vector2()

puppet var slave_position = Vector2()

func set_id_name(playerId):
	$Label.set_text(playerId)
	
func _ready():
	pass
	
func MovePlayer(input):
	match input:
		RIGHT:
			velocity.x =  WALK_SPEED
		LEFT:
			velocity.x = -WALK_SPEED
		UP:
			velocity.y =  -WALK_SPEED
		DOWN:
			velocity.y = WALK_SPEED
		_:
			velocity.y = 0
			velocity.x = 0
	move_and_slide(velocity)
	position.x = round(position.x)
	position.y = round(position.y)
	
	 
#		
	
