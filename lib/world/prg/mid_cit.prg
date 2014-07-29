>entry_prog 30~
    emote keeps an eye out for wanted criminals.
~
>greet_prog 40~
look $n
if isgood($n)
	say Good day.
	smile $n
else
	mpechoaround $n $I wonders if $n is among the most wanted.
	mpechoat $n $I wonders if you are among the most wanted.
endif
~
>rand_prog 20~
if rand(60)
	emote whistles a little song.
else
	emote pulls something out of $l nose. 
endif
~
>act_prog p bursts into~
say What is wrong?
~
>act_prog p rolls down~
say What's so funny?
~
>act_prog p bandit arrives~
if rand(50)
	shout Bandit just passed $Z!
    if rand(35)
        shout What shall I do?
        if rand(25)
             mpat yoda mpechoat yoda bandit_trig
        endif
    endif
else
	shout I saw the bandit at $Z!
endif
~
>act_prog p kill_bandit~
shout Right! BANZAAAAI!
mpkill bandit
~
>speech_prog p My girlfriend left me~
comfort $n 
~
|

