extends KinematicBody2D


#const GRAVITY = 200.0
const WALK_SPEED = 80
const RIGHT = 1
const LEFT = 2
const UP = 3
const DOWN = 4


var velocity = Vector2()

puppet var slave_position = Vector2()

var playerID = ""

func set_id_name(playerId):
	$Label1.set_text(playerId)
	
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
		#Examples 
		
#		#JUMP:
#			velocity.y = jump_speed
#		ATTACK:
#			JAB:
#				[insert animation]
#			PROJECTILE:
#			   	Fireball.active = true
#               Fireball.position.x = Fireball.position.x
#               Fireball.position.y = Fireball.position.y
#               Fireball.velocity.dx = Fireball.velocity.x 
#               Fireball.velocity.dy = Fireball.velocity.x 
#               Fireball.cooldown = BULLET_COOLDOWN
	move_and_slide(velocity)
	position.x = round(position.x)
	position.y = round(position.y)
