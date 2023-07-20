//#include <xmodem.h>

struct midi_note {
  uint8_t type;
  uint8_t channel;
  uint8_t note;
  uint8_t velocity;
};

const uint8_t LED = 13;
const uint8_t SPEAKER_PIN = 8;
uint8_t led = 0;
#include "/home/pg_mana/Develop/build/Arduino/midi-player/test.txt"

//uint8_t buf[2048];
uint32_t rp = 0, wp = sizeof(buf), available_size = sizeof(buf);


void setup() {
  // put your setup code here, to run once:
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  while (!Serial);
  Serial.write("Ready for read the MIDI(SMF) File!\n");
  digitalWrite(LED, led);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t data[10];
  while (available_size < 4)write_into_buf(buf, sizeof(buf), &available_size, &wp);
  set_array_from_buf(data, 4, buf, sizeof(buf), &available_size, &rp);

  if (!check_smf_signature(data)) {
    Serial.write(data, 4);
    return;
  }
  while (available_size < 10)write_into_buf(buf, sizeof(buf), &available_size, &wp);
  set_array_from_buf(data, 10, buf, sizeof(buf), &available_size, &rp);

  if (!check_size(data)) {
    tone(SPEAKER_PIN, 200);
    Serial.write("Invalid Header Size!\n");
    return;
  }
  else if (!check_format(data + 4)) {
    tone(SPEAKER_PIN, 300);
    Serial.write("Invalid Format Version!\n");
    return;
  } else if (!check_is_single_track(data + 6)) {
    Serial.write("Multi track is not supported...\n");
    tone(SPEAKER_PIN, 400);
    return;
  }
  uint16_t resolution = get_resolution(data + 8);
  if (resolution == 0) {
    tone(SPEAKER_PIN, 500);
    Serial.write("Not supported resolution...\n");
  }
  if (!play_loop(resolution)) {
    tone(SPEAKER_PIN, 600);
    Serial.write("Failed to play...\n");
  }
}




bool play_loop(const uint16_t resolution) {
  uint32_t track_size = 0;
  uint64_t delta_time = 0;
  uint32_t tempo = 0;
  uint8_t processing_event_type = 0;
  struct midi_note note;
  bool maybe_runnig_status = false;
  bool processed_midi_note = false;

  while (true) {
    write_into_buf(buf, sizeof(buf), &available_size, &wp);
    if (track_size == 0) {
      uint8_t data[4];
      while (available_size < 4) {
        write_into_buf(buf, sizeof(buf), &available_size, &wp);
      }
      set_array_from_buf(data, 4, buf, sizeof(buf), &available_size, &rp);
      if (!check_chunk_signature(data)) {
        Serial.write(data, 4);
        Serial.write("\nTrack Signature is not correct!\n");
        return false;
      }
      while (available_size < 4) {
        write_into_buf(buf, sizeof(buf), &available_size, &wp);
      }
      set_array_from_buf(data, 4, buf, sizeof(buf), &available_size, &rp);
      track_size = get_chunk_size(data);
    }

    /*Delta time*/
    delta_time = 0;
    while (true) {
      while (available_size == 0) {
        write_into_buf(buf, sizeof(buf), &available_size, &wp);
      }
      if (get_delta_time(read_from_buf(buf, sizeof(buf), &available_size, &rp), &delta_time)) {
        break;
      }
    }

    /*Event Switch*/
    if (processing_event_type == 1 || (processing_event_type == 0 && is_midi_event(fetch_from_buf(buf, sizeof(buf), &available_size, &rp)))) {
      processed_midi_note = true;
      maybe_runnig_status = true;
      uint8_t data[3];
      if (processing_event_type == 0) {
        data[0] = read_from_buf(buf, sizeof(buf), &available_size, &rp);
        processing_event_type = 1;
      }
      while (available_size < 2) {
        write_into_buf(buf, sizeof(buf), &available_size, &wp);
      }
      set_array_from_buf(data + 1, 2, buf, sizeof(buf), &available_size, &rp);

      note = get_midi_note(data);

      uint64_t delta_us = (double)(tempo) * ((double)delta_time / (double)resolution);

      if (note.type == 8 || (note.type == 9 && note.velocity == 0)) {
        /*note off*/
        if (delta_us != 0) {
          if (delta_us >= 1000) {
            delay(delta_us / 1000);
          } else {
            delayMicroseconds(delta_us);
          }
        }
        noTone(SPEAKER_PIN);
        digitalWrite(LED, led);
        led ^= 1;
      } else if (note.type == 9) {
        /*note on*/
        if (delta_us != 0) {
          if (delta_us >= 1000) {
            delay(delta_us / 1000);
          } else {
            delayMicroseconds(delta_us);
          }
        }
        //uint64_t hz = get_base_frequency_mhz(note.note % 12) * (note.note / 12) / 100;
        uint64_t hz = 440 * pow(2, ((double)(note.note) - 69) / 12);
        tone(SPEAKER_PIN, hz);
        digitalWrite(LED, led);
        led ^= 1;
      } else {
      }
      processing_event_type = 0;
    } else if (processing_event_type == 2 || (processing_event_type == 0 && is_sys_ex_event(fetch_from_buf(buf, sizeof(buf), &available_size, &rp)))) {
      maybe_runnig_status = false;
      if (processing_event_type == 0) {
        read_from_buf(buf, sizeof(buf), &available_size, &rp); //typeの読み捨て
        processing_event_type = 2;
      }
      uint64_t data_size = 0;
      while (true) {
        while (available_size == 0) {
          write_into_buf(buf, sizeof(buf), &available_size, &wp);
        }
        if (get_sys_event_size(read_from_buf(buf, sizeof(buf), &available_size, &rp), &data_size)) {
          break;
        }
      }
      while (data_size--) {
        while (available_size == 0) {
          write_into_buf(buf, sizeof(buf), &available_size, &wp);
        }
        read_from_buf(buf, sizeof(buf), &available_size, &rp);/*throw away*/
      }
      processing_event_type = 0;
    } else if (processing_event_type == 3 || (processing_event_type == 0 && is_meta_event(fetch_from_buf(buf, sizeof(buf), &available_size, &rp)))) {
      maybe_runnig_status = false;
      if (processing_event_type == 0) {
        read_from_buf(buf, sizeof(buf), &available_size, &rp); //typeの読み捨て
        processing_event_type = 3;
      }
      uint8_t meta_data_type = 0;
      while (available_size == 0) {
        write_into_buf(buf, sizeof(buf), &available_size, &wp);
      }
      meta_data_type  = read_from_buf(buf, sizeof(buf), &available_size, &rp);

      uint64_t data_size = 0;
      while (true) {
        while (available_size == 0) {
          write_into_buf(buf, sizeof(buf), &available_size, &wp);
        }
        const uint8_t t = read_from_buf(buf, sizeof(buf), &available_size, &rp);
        if (get_meta_data_size(t, &data_size)) {
          break;
        }
      }
      switch (meta_data_type) {
        case 0x51: {
            /*tempo*/
            uint8_t temp[3];
            while (available_size < 3) {
              write_into_buf(buf, sizeof(buf), &available_size, &wp);
            }
            set_array_from_buf(temp, 3, buf, sizeof(buf), &available_size, &rp);

            tempo = get_tempo(temp);
            break;
          }
        case 0x2f: {
            if (processed_midi_note) {
              noTone(SPEAKER_PIN);
              return true;
            } else {
              /*maybe meta track*/
              track_size = 0;
            }
            /*end of truck*/
            break;
          }
        default: {
            while (data_size--) {
              while (available_size == 0) {
                write_into_buf(buf, sizeof(buf), &available_size, &wp);
              }
              read_from_buf(buf, sizeof(buf), &available_size, &rp);/*throw away*/
            }
            break;
          }
      }
      processing_event_type = 0;
    } else {
      if (!maybe_runnig_status) {
        //tone(SPEAKER_PIN, 300);
        char string[20];
        uint8_t data[3] = {0};
        set_array_from_buf(data, 3, buf, sizeof(buf), &available_size, &rp);
        sprintf(string, "\nUnknown:%x %x %x\n", data[0], data[1], data[2]);
        Serial.write(string);
        return false;
      }
      /*running status*/
      while (available_size < 2) {
        write_into_buf(buf, sizeof(buf), &available_size, &wp);
      }
      uint8_t data[2];
      set_array_from_buf(data, 2, buf, sizeof(buf), &available_size, &rp);
      update_midi_note(&note, data);
      uint64_t delta_us = (double)(tempo) * ((double)delta_time / (double)resolution);

      if (note.type == 8 || (note.type == 9 && note.velocity == 0)) {
        /*note off*/
        if (delta_us != 0) {
          if (delta_us > 1000) {
            delay(delta_us / 1000);
          } else {
            delayMicroseconds(delta_us);
          }
        }
        noTone(SPEAKER_PIN);
        digitalWrite(LED, led);
        led ^= 1;
      } else if (note.type == 9) {
        if (delta_us != 0) {
          if (delta_us > 1000) {
            delay(delta_us / 1000);
          } else {
            delayMicroseconds(delta_us);
          }
        }
        /*note on*/
        //uint64_t hz = get_base_frequency_mhz(note.note % 12) * (note.note / 12) / 100;
        uint64_t hz = 440 * pow(2, ((double)(note.note) - 69) / 12);
        tone(SPEAKER_PIN, hz);
        digitalWrite(LED, led);
        led ^= 1;
      } else {

      }

    }
  }
}

void write_into_buf(uint8_t buf[], size_t buf_size, uint32_t *available_size, uint32_t *wp) {
  return;
  while ( Serial.available()) {
    buf[(*wp)++] = Serial.read();
    *available_size += 1;
    if (*available_size == buf_size) {
      break;
    }
    if (*wp == buf_size) {
      *wp = 0;
    }
  }
}

uint8_t fetch_from_buf(const uint8_t buf[], size_t buf_size, uint32_t *available_size, uint32_t *rp) {
  if (*available_size == 0) {
    return 0;
  }
  return pgm_read_byte_near(buf + *rp);//buf[*rp];
}

uint8_t read_from_buf(const uint8_t buf[], size_t buf_size, uint32_t *available_size, uint32_t *rp) {
  if (*available_size == 0) {
    return 0;
  }
  uint8_t data = fetch_from_buf(buf, buf_size, available_size, rp);
  *rp += 1;
  if (*rp == buf_size) {
    *rp = 0;
  }
  return data;
}

void set_array_from_buf(uint8_t buf_to_write[], const size_t read_size, const uint8_t data_buf[], const size_t data_buf_size, uint32_t *available_size, uint32_t *rp) {
  for (size_t i = 0; i < read_size; i++) {
    buf_to_write[i] = read_from_buf(data_buf, data_buf_size, available_size, rp);
  }
}

bool check_smf_signature(const uint8_t buf[4]) {
  return buf[0] == 0x4d && buf[1] == 0x54 && buf[2] == 0x68 && buf[3] == 0x64; /* MThd */
}

bool check_size(const uint8_t buf[4]) {
  return buf[0] == buf[1] == buf[2] == 0 && buf[3] == 0x06;
}

bool check_format(const uint8_t buf[2]) {
  return buf[0] == 0 && (buf[1] == 0 || buf[1] == 0x01);
}

bool check_is_single_track(const uint8_t buf[2]) {
  return true;
  return buf[0] == 0 && (buf[1] == 0x01 || buf[1] == 0x02);
}

uint16_t get_resolution(const uint8_t buf[2]) {
  if (buf[0] >= 0x80) {
    return 0;
  }
  return (uint16_t)(buf[0]) << 8 | (uint16_t)buf[1];
}

bool check_chunk_signature(const uint8_t buf[4]) {
  return buf[0] == 0x4d && buf[1] == 0x54 && buf[2] == 0x72 && buf[3] == 0x6b;
}

uint32_t get_chunk_size(const uint8_t buf[4]) {
  /*エンディアンなぁ*/
  return ((uint32_t)(buf[0]) << 8 * 3) | ((uint32_t)(buf[1]) << 8 * 2) | ((uint32_t)(buf[2]) << 8 * 1) | ((uint32_t)buf[3]);
}


bool get_delta_time(const uint8_t data, uint64_t *result) {
  *result <<= 7;
  *result |= (data & 0x7f);
  if (data < 0x80) {
    return true;
  } else {
    return false;
  }
}

bool is_event_data(const uint8_t data) {
  return (1 << 7) & data != 0;
}

bool is_midi_event(const uint8_t data) {
  const uint8_t high = data >> 4;
  return high == 0x8 || high == 0x9 || high == 0xa || high == 0xb || high == 0xc || high == 0xd || high == 0xe;
}

bool is_sys_ex_event(const uint8_t data) {
  return data == 0xf7 || data == 0xf0;
}

bool get_sys_event_size(const uint8_t data, uint64_t *result) {
  return get_delta_time(data, result);
}

bool is_meta_event(const uint8_t data) {
  return data == 0xff;
}

uint8_t get_meta_event_type(const uint8_t data) {
  return data;
}

bool get_meta_data_size(const uint8_t data, uint64_t *result) {
  return get_delta_time(data, result);
}

uint32_t get_tempo(const uint8_t buf[3]) {
  return (((uint32_t)buf[0]) << (8 * 2)) | (((uint32_t)buf[1]) << 8) | ((uint32_t)buf[2]);
}


struct midi_note get_midi_note(const uint8_t buf[3]) {
  struct midi_note result;
  result.type = buf[0] >> 4;
  result.channel = buf[0] & 0xf;
  result.note = buf[1];
  result.velocity = buf[2];
  return result;
}

void update_midi_note(struct midi_note * note, const uint8_t data[2]) {
  note->note = data[0];
  note->velocity = data[1];
}

/*you must div by 1000 to convert to hz*/
/*uint64_t get_base_frequency_mhz(const uint32_t base) {
  switch (base) {
    case 0://C-1
      return 8175;
    case 1: //C#-1
      return 8662;
    case 2: //D-1
      return 9177;
    case 3://D#-1
      return 9722;
    case 4://E-1
      return 10301;
    case 5://F-1
      return 10913;
    case 6://F#-1
      return 11562;
    case 7://G-1
      return 12250;
    case 8://G#-1
      return 12978;
    case 9://A-1
      return 13750;
    case 10://A#-1
      return 14568;
    case 11://B-1
      return 15434;
    default:
      return 0;
  }
  }*/
