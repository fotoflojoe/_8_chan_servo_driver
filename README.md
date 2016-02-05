# _8_chan_servo_driver

Application: Six channel servo driver (Ardiuno Sketch)

Purpose: Take input from momentary pushbutton (NO), move servo between two set positions, at set speed. Provide visual feedback as to servo's position using LEDs (green/hi, red/low).  


sketch setup()
  - initialize servos and feedback LEDs to set start position (range high)


  
sketch loop()
  - Read new input shift register values (74HC165)
  - Create for loop, bounded by number_of_servos; index i
    - if new input register position i is high and old input register position is !high (valid button press)
      - if servo i is range high (straight route)
        - move servo i to range low (divergent route)
        - set signal position i (74HC595) to range low
      - else
        - move servo i to range high (straight route)
        - set signal position i (74HC595) to range high
    - else
      - set servo i detach value
  - set old input shift register values from new input shift register values 
