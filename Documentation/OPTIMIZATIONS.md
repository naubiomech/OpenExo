## Optimizations
- SPI direct memory access. Should change SPI time from 20 &mu;s per byte to the time to write the memory.
    - Teensy Datasheet 48.2.1 
        - Configure SPI
        - Teensy as controller and Nano as peripherial.
    - At top of loop teensy
        - Teensy flushes TX buffer and places fresh data data on TX Buffer.
        - Read if there are new messages from Nano and que completed messages.  Nano will read fresh data when this happens.
        - Teensy processes the que of new complete messages.  This should be a couple of messages and should be fine as the data sent will likely be smaller than data received.
        - Teensy runs the rest of the control loop.
    - Nano would place messages on tx buffer as bluetooth messages come in.
        - If the Nano is placing data when the Teensy reads the message will be sent over incomplete and will need to place the rest of the message afterwards which the Teensy will append when the next read happens.
        - Will need a message complete indicator at end of message.
        - Teensy que will also need a message complete flag to know if it needs to precess the message or if the message is incomplete start the next read by appending the rest of the message here. 
        
        
- BLEparser change from char representation to num bytes, expect 3 to 6x speed up.
    - This should be pretty straight forward as it can pretty much follow how messages are packed for the SPI.
    - I am not sure what issues will arise on the app side.
        - On the app you could just add a converter from num to char if you want to be lazy, but presumably you already have a char to num converter so I would recommend just cutting that out and fixing the stuff that breaks.
    
- CAN direct memory access, should cut about 250 &mu;s per motor.  Will still need a loop period of at least 250 &mu;s for the motors to return data.
    - https://www.pjrc.com/teensy/IMXRT1060RM_rev3.pdf 
        - CH 44
    - 6 mailboxes between TX and RX may have issues for a 4 motor setup without using a CAN bus per leg.  
        - All the complication with the limited mailboxes can be simplified by just using separate CAN buses for each leg, see other optimization below.  I have included how you could do it by switching mailboxes below, but don't do that if you don't have to.
            - Setup and Debug this with a bilateral uni-joint or unilateral multi-joint system so you can keep the motor count below 3 so you don't have to fiddle with all the mailbox shifting.  Then just setup the CAN2 below.
        - Can potentially use an ISR to mitigate this with a bit of loss in performance but will still be a big jump from where we are.
        - Can potentially switch mailbox type after emptied. Pair first TX (44.7.2) with first RX(44.7.4). When first TX is cleared becomes RX for next TX, or similar.  **DON'T WRITE A TX WITHOUT A CORESPONDING RX**.  You can write based on priority to help, highest priority first so you know it will return first.
            - This doesn't have to be first TX becomes second RX but you can play around to see what works.  Could have two TX RX pairs and two unpaired TX then the first two TX are emptied, change them to the RX for the unpaired TX and fill the second set of TX for writing.
        - 44.7.6 will also be useful.  In addition to the registers/memory map at the end.
    
- CAN bus per leg after DMA, roughly cut CAN time in half. Main control may be limiting time at this point at about 600&mu;s. Write at top, do controls, read at bottom.
    - Basically just follow above but with CAN2.  
    - Will need to update CAN code to make the CAN selectable 
    - Will need to attach a CAN object to the Leg instance rather than the Exo instance.