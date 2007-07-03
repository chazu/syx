"
Copyright (c) 2007 Luca Bruno

This file is part of Smalltalk YX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
"

!Stream methodsFor: 'accessing-reading'!

next
    self subclassResponsibility
!


contents
    "Answer the contents of the stream from current position on"
    | stream |
    stream := WriteStream on: (self collectionClass new: 10).
    self do: [ :each | stream nextPut: each ].
    ^stream contents
! !


!Stream methodsFor: 'accessing-writing'!   

nextPut: anObject   
    "Write anObject to the receiver"
    self subclassResponsibility
!


nextPutAll: aCollection
    "Write all the objects in aCollection to the receiver"
    aCollection do: [ :element | self nextPut: element ].
    ^aCollection
! !

!Stream methodsFor: 'testing'!

atEnd
    "Return true if i'm at the end"
    self subclassResponsibility
! !

!Stream methodsFor: 'enumerating'!

do: aBlock
    "Call a block for each element of the stream from the current position on, until the end."
    [ self atEnd ] whileFalse:
        [ aBlock value: self next ]

! !


!Stream methodsFor: 'basic'!

collectionClass
    "Answer the collection class that's used to return the contents of the stream"
    ^Array
!

nl
    "Write a new line"
    self nextPut: Character nl
!

cr 
    "Write a carriage return"
    self nextPut: Character cr
! !