code

export vmMain
proc vmMain 0 0
ADDRGP4 leaky
CALLI4
pop
CNSTI4 0
RETI4
LABELV $1
endproc vmMain 0 0

export leaky
proc leaky 0 0
CNSTI4 5
CNSTI4 6
CNSTI4 7
CNSTI4 0
RETI4
LABELV $2
endproc leaky 0 0
