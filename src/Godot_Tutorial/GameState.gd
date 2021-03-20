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
		
#		Examples:
#		p.PlayerAttack()
#       p.PlayerHurt() 
#       p.MoveAI()

		i += 1

func Save_GameState():
	var stream = StreamPeerBuffer.new()
	stream.clear()
	stream.put_32(0) # Reserves space for the checksum 
	for i in players:
		
	
#		put_8 range = -128 to 127
#		put_16 range = −32,768 to 32,767 
#		put_32 range = -2147483648 to 2147483647
#		put_64 range = −(2^63) to 2^63 − 1	

		
		
		stream.put_16(round(i.velocity.x)) # 4
		stream.put_16(round(i.velocity.y)) # 5
		stream.put_16(round(i.position.x)) # 331
		stream.put_16(round(i.position.y)) # 91
		#stream.put_string("test") 

	var check = CalcFletcher32(stream)
	stream.seek(0)
	stream.put_32(check)
	
	stream.seek(0)
	stream.get_32() 
	var dx = stream.get_8() # 4
	var dy = stream.get_8() # 5
	var px = stream.get_16() # 331
	var py = stream.get_16() # 91

	print("[" + str(dx) + "," + str(dy) + "," + str(px) + "," + str(py) + "] - Saved")
#	stream.seek(0)
	return stream

func Load_GameState(buffer):
	buffer.seek(0) # Sets the position indicator for the StreamPeerBuffer to 0.
	buffer.get_32() # Removes the checksum 
	print("[" + str(buffer.get_8()) + "," +str(buffer.get_8()) + "," + str(buffer.get_16()) + "," + str(buffer.get_16()) + "] - Loaded")
	
	buffer.seek(0) # Sets the position indicator for the StreamPeerBuffer to 0.
	buffer.get_32() # Removes the checksum 
	for p in players:
		p.velocity.x = buffer.get_16() # 4
		p.velocity.y = buffer.get_16() # 5
		p.position.x = buffer.get_16() # 331
		p.position.y = buffer.get_16() # 91
		#p.set_id_name(buffer.get_string()) # "test"


func CalcFletcher32(data):
    var sum1 = 0
    var sum2 = 0
    var index = data.get_data_array()
    for i in range(index.size()):
        sum1 = (sum1 + index[i]) % 0xffff
        sum2 = (sum2 + sum1) % 0xffff
    
    return (int(sum2 << 16) | sum1)
