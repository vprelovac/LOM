>rand_prog 60~
if rand(70)
	dance
else
	emote sings, 'We're drinking beer, we're drink gin...'
    emote sings, 'No matter what's in bottle, we're drinking everything..'
endif
~
>greet_prog 10~
if isnpc($n)
yell Monster!  I found a monster!  Kill!  Banzai!
mpkill $n
endif
~
>bribe_prog 10~
say Ahh!  More spirits!  Good spirits!
sing
~
|
