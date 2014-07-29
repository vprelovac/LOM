>hitprcnt_prog 30~
	holl MUHAHAHAHA!
	wie scythe
~
>rand_prog 40~
if rand(20)
    say If you say give me life, I'll do it... I know I'll see you again
    grin
else if rand(50)
    emote slowly waves it's scythe. 
endif
~
>speech_prog p give me life~
look $n
if rand(50)
    say Hope to see you around...
else
    if rand(50) 
        say My doors are always open for you...
    else
        say Come again...
    endif
endif
giggle
emote pulls a blue lever.
mpechoat $n You are briefly surrounded by a thick smoke.
mptransfer $n 3001 1
~
|
