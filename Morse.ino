/*
  Morse Coder

  Uses the LED to transmit Morse Code.
  
  '.' dot
  '-' dash
  ',' charcter delimiter
  '_' letter delimiter
  ' ' word delimiter

  created 2024
  by Ogoruwa

  This code is licensed under the MIT license.
*/

/* Dictionary errors 
  0: success,
  -1: General,
  -2: Memory allocation,
  -3: comppression input size,
  -20: comma expected,
  -21: colon expected,
  -22: quote expected,
  -23: backslash expected,
  -99: unexpected end of string
*/


#include <Dictionary.h>
#include <DictionaryDeclarations.h>


#undef _DICT_CRC
#undef _DICT_KEYLEN
#undef _DICT_VALLEN

#define _DICT_CRC 16
#define _DICT_KEYLEN 1
#define _DICT_VALLEN 8


#define IGNORE 0
#define ERROR 1
#define INFO 2
#define DEBUG 3
#define VERBOSE 4
#define LOG_LEVEL ERROR



int get_free_sram() {
  // Get free stack RAM
  int v;
  extern int __heap_start, *__brkval;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}



void display_free_sram() {
  // Prints free Stack RAM
  Serial.print(F("- SRAM left: "));
  Serial.println(get_free_sram());
}



void encode_character(char character, char encoded[]) {
  // Encodes a character as Morse code
  const short length = 28;
  static Dictionary *mapping_characters = new Dictionary(length);

  if (mapping_characters->key(0) == "") {
    const char characters[length] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' ', '\n' };
    const char *codes[length] = { ".-", "-...", "-.-.", "-...", ".", "..-.", "--.", "....", "..", ".---", "-.-", "..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", " ", ".-.-." };

    if (LOG_LEVEL >= INFO) {
      Serial.println(F("Characters mapping ..."));
    }

    for (short i = 0; i < length; i++) {
      const char character[2] = { characters[i], '\0' };
      const short error = mapping_characters->insert(character, codes[i]);

      if (error && LOG_LEVEL >= ERROR) {
        Serial.print(F("Characters mapping error: "));
        Serial.println(error);
        break;
      }
    }
  }
  
  char a = tolower(character);
  char b[2] = {a, '\0'};
  String code = mapping_characters->search(b);

  for (short i = 0; i < code.length(); i++) {
    encoded[i] = code.charAt(i);
  }
}



int transmit(char code, const short pins[], const short pins_length) {
  // Transmits Morse code as digital signals
  const unsigned short length = 5;

  // Symbols: code delimiter (dit), dit, dah (3*dit), letter delimiter (dah), word delimiter (7*dit)
  const char symbols[length] = { ',', '.', '-', '_', ' ' };
  static Dictionary *mapping_states = new Dictionary(length);
  static Dictionary *mapping_durations = new Dictionary(length);

  {
    if (mapping_durations->search(".") == "") {
      const unsigned int duration_dit = 300;
      const unsigned int durations[length] = { duration_dit, duration_dit, duration_dit * 3, duration_dit * 1, duration_dit * 5 };

      if (LOG_LEVEL >= INFO) {
        Serial.println(F("Durations mapping ..."));
      }

      for (short i = 0; i < length; i++) {
        char array[6];
        char symbol[2] = { symbols[i], '\0' };
        itoa(durations[i], array, 10);
        const short error = mapping_durations->insert(symbol, array);

        if (error && LOG_LEVEL >= ERROR) {
          Serial.print(F("Durations mapping error: "));
          Serial.println(error);
          break;
        }
      }
    }
  }

  {
    if (mapping_states->search(".") == "") {
      const bool states[length] = { LOW, HIGH, HIGH, LOW, LOW };

      if (LOG_LEVEL >= INFO) {
        Serial.println(F("States mapping ..."));
      }

      for (short i = 0; i < length; i++) {
        char array[2];
        char symbol[2] = { symbols[i], '\0' };
        itoa(states[i], array, 10);
        const short error = mapping_states->insert(symbol, array);

        if (error && LOG_LEVEL >= ERROR) {
          Serial.print(F("States mapping error: "));
          Serial.println(error);
          break;
        }
      }
    }
  }

  const char a[2] = { code, '\0' };

  String i = mapping_states->search(a);
  if (i.c_str() == "") {
    return 0;
  }
  const bool pin_state = i.toInt();

  i = mapping_durations->search(a);
  if (i.c_str() == "") {
    return 0;
  }
  const int duration = i.toInt();

  if (LOG_LEVEL >= DEBUG) {
    Serial.print(F("Code: "));
    Serial.print(a);
    Serial.print(F(", "));
    Serial.print(duration);
    Serial.print(F(" ms, "));
    Serial.println(pin_state ? "HIGH" : "LOW");
  }

  for (short i = 0; i < pins_length; i++ ) {
    const short pin = pins[i];
    digitalWrite(pin, pin_state);
  }

  return duration;
}



void parse_input(char buffer[], short input_length) {
  while (Serial.available() && strlen(buffer) < input_length) {
    const char i = Serial.read();
    const char input[2] = { i, '\0' };

    if (strlcat(buffer, input, input_length) >= input_length) {
      if (LOG_LEVEL >= ERROR) {
        Serial.println("Input overflow ...");
      }
      break;
    }
  }
}



void morse(long delta, char input[]) {
  // Takes a character array, encodes and transmits it as Morse code
  static char encoded[9];
  static bool send = false;
  static bool encode = true;
  static int code_duration = 0;
  const short pins[] = { LED_BUILTIN };
  const short pins_length = sizeof(pins);

  short length = strlen(input);
  // Encode first chracter in input if code array is empty and input is not
  if (encode && length > 0) {
    const char character = input[0];
    encode_character(character, encoded);
    short l = strlen(encoded);

    // Fill the ununsed code array slots with null termminator
    for (short i = l; i < sizeof(encoded); i++) {
      encoded[i] = '\0';
    }

    // Move values in the input to the left, effectively removing the first value
    for (short i = 0; i < length - 1; i++) {
      input[i] = input[i + 1];
    }

    send = true;
    encode = false;
    input[length - 1] = '\0';
  }

  if (send) {
    if (code_duration <= 0) {
      char code;
      static bool comma = false;
      static bool letter = false;
      const short l = strlen(encoded);

      if (comma) {
        code = ',';

        if (letter) {
          encode = true;
          send = letter = false;
        } else if (l == 0) {
          letter = true;
        }
      }

      else if (letter) {
        code = '_';
      }

      else {
        code = encoded[0];

        // Move values in code array to the left, effectively removing the first value
        for (short j = 0; j < l; j++) {
          encoded[j] = encoded[j + 1];
        }
        encoded[l - 1] = '\0';
      }

      comma = !comma;
      code_duration = transmit(code, pins, pins_length);

      if (LOG_LEVEL >= ERROR && code_duration == 0) {
        Serial.print(code);
        Serial.println(F(" not valid code character"));
      }

    } else {
      code_duration -= delta;
    }
  }
}


/* Arduino Functions */

void setup() {
  const short pins[] = { LED_BUILTIN };

  for (short i = 0; i < sizeof(pins) / sizeof(pins[0]); i++ ) {
    const short pin = pins[i];
    digitalWrite(pin, LOW);
  }

  Serial.begin(9600);
  Serial.println(F("\n\n----- Starting Morse Coder -----"));
}


void loop() {
  static bool receive = true;
  static char input[65] = { "hello world\nsos\n" };
  static unsigned long previous_time = 0;
  const unsigned long current_time = millis();
  const unsigned long delta = current_time - previous_time;
  previous_time = current_time;

  if (receive) {
    parse_input(input, sizeof(input));
  }


  if (LOG_LEVEL >= VERBOSE) {
    Serial.print(F("Delta: "));
    Serial.print(delta);
    Serial.println(F("ms "));
  }

  morse(delta, input);
}


/* END Arduino Functions */
