

// Editor
// https://ledmatrix-editor.arduino.cc/
// 
// Info
// https://docs.arduino.cc/tutorials/uno-r4-wifi/led-matrix/
//
// 12x8 Matrix


// --------------------------------------------------------------------------------

const int MAX_MATRIX = 10;

String matrixNames[MAX_MATRIX];
uint32_t* matrixValues[MAX_MATRIX];
int matrixCount = 0;


void setMatrix(String name, uint32_t* value) {
  for (int i = 0; i < matrixCount; i++) {
    if (matrixNames[i] == name) {
      matrixValues[i] = value;
      return;
    }
  }
  // Neue Variable anlegen
  if (matrixCount < MAX_MATRIX) {
    matrixNames[matrixCount] = name;
    matrixValues[matrixCount] = value;
    matrixCount++;
  }
}

uint32_t* getMatrix(String name) {
  for (int i = 0; i < matrixCount; i++) 
	{
    if (matrixNames[i] == name) return matrixValues[i];
  }
  return 0; // Standardwert
}

// --------------------------------------------------------------------------------

const uint32_t matrix_arduino[] = {
		0x31c4a284,
		0x9b5d8494,
		0xa231c000
};


const uint32_t matrix_happy[] = {
    0x19819,
    0x80000001,
    0x81f8000
};

const uint32_t matrix_heart[] = {
    0x3184a444,
    0x44042081,
    0x100a0040
};

const uint32_t matrix_empty[] = {
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t matrix_full[] = {
    0xffffffff,
    0xffffffff,
    0xffffffff
};

const uint32_t matrix_line1[] = {
		0xaaa555aa,
		0xa555aaa5,
		0x55aaa555
};

const uint32_t matrix_line2[] = {
		0x555aaa55,
		0x5aaa555a,
		0xaa555aaa
};

const uint32_t matrix_karo1[] = {
		0xf0ff0ff0,
		0xff0f0f00,
		0xf00f00f0
};

const uint32_t matrix_karo2[] = {
		0xf00f00f,
		0x0f0f0ff,
		0xff0ff0f
};

const uint32_t matrix_ok[] = {
		0xfff801b9,
		0x5a99a99b,
		0x95801fff
};

const uint32_t matrix_herz[] = {
		0x30c79eff,
		0xf7fe3fc1,
		0xf80f0060
};

const uint32_t matrix_herzklein[] = {
		0x30c79eff,
		0xf7fe3fc1,
		0xf80f0060
};

const uint32_t matrix_herzrand[] = {
		0x30c79ecf,
		0x360630c1,
		0x980f0060
};

const uint32_t matrix_hackerl[] = {
		0xfff80180,
		0x98118a18,
		0x41801fff
};

const uint32_t matrix_box_0[] = {
		0xfff80180,
		0x18018018,
		0x01801fff
};

const uint32_t matrix_box_1[] = {
		0xfff801a0,
		0x1a01a01a,
		0x01801fff
};

const uint32_t matrix_box_2[] = {
		0xfff801b0,
		0x1b01b01b,
		0x01801fff
};
const uint32_t matrix_box_3[] = {
		0xfff801b8,
		0x1b81b81b,
		0x81801fff
};

const uint32_t matrix_box_4[] = {
		0xfff801bc,
		0x1bc1bc1b,
		0xc1801fff
};

const uint32_t matrix_box_5[] = {
		0xfff801be,
		0x1be1be1b,
		0xe1801fff,
};

const uint32_t matrix_box_6[] = {
		0xfff801bf,
		0x1bf1bf1b,
		0xf1801fff
};

const uint32_t matrix_box_7[] = {
		0xfff801bf,
		0x9bf9bf9b,
		0xf9801fff
};

const uint32_t matrix_box_8[] = {
		0xfff801bf,
		0xdbfdbfdb,
		0xfd801fff
};

const uint32_t matrix_box_inv[] = {
		0x0007fe40,
		0x24024024,
		0x027fe000
};

const uint32_t matrix_net[] = {
		0x00977d4,
		0x2d62b42b,
		0x42972000,
};

const uint32_t matrix_init[] = {
		0x0000a57b5,
		0x2b52ad2a,
		0xd2a52000,
};

const uint32_t matrix_sensoren[] = {
		0x00066988,
		0xd8cd48b2,
		0x8bc69000
};

const uint32_t matrix_mqtt[] = {
		0x00088ed9,
		0x1a91a918,
		0x9288d000
};

const uint32_t matrix_pin[] = {
		0x000e5195,
		0x9955e538,
		0x51851000
};

const uint32_t matrix_pfeil1[] = {
		0x0c00f00f,
		0xcffffff0,
		0xfc0f00c0
};
const uint32_t matrix_pfeil2[] = {
		0x0880cc0e,
		0xeffffff0,
		0xee0cc088
};

const uint32_t matrix_pfeil3[] = {
		0x00044466,
		0x6ffffff6,
		0x66444000
};


const uint32_t matrix_music[] = {
		0x00400650,
		0x7f85505f,
		0x9c53c018
};

const uint32_t matrix_note2[] = {
    0x1041861c,
		0x71451457,
		0x1cf3c618
};

const uint32_t matrix_note1[] = {
		0x002003003,
		0x80280280,
		0xe01e00c0
};

const uint32_t matrix_note_loch[] = {
		0x02003003,
		0x80280280,
		0xe01200c0
};

const uint32_t matrix_lcd[] = {
		0x0008ee88,
		0x98898898,
		0x89eee000
};

const uint32_t matrix_clock_leer[] = {
		0x0f010820,
		0x42042042,
		0x041080f0
};

const uint32_t matrix_clock_1[] = {
		0x0f010824,
		0x42442642,
		0x041080f0
};

const uint32_t matrix_clock_2[] = {
		0x0f010820,
		0x42742442,
		0x041080f0
};

const uint32_t matrix_clock_3[] = {
		0x0f010820,
		0x42642242,
		0x241080f0
};

const uint32_t matrix_clock_4[] = {
		0x0f010820,
		0x42242e42,
		0x041080f0
};
		
void setup_symbols()
{
  setMatrix("arduino", (uint32_t*)matrix_arduino);
  setMatrix("ok",      (uint32_t*)matrix_ok);
  setMatrix("mqtt",    (uint32_t*)matrix_mqtt);
  setMatrix("net",     (uint32_t*)matrix_net);	  
  setMatrix("init",    (uint32_t*)matrix_init);
  setMatrix("note1",   (uint32_t*)matrix_note1);
  setMatrix("clock1",  (uint32_t*)matrix_clock_1);
  setMatrix("lcd",     (uint32_t*)matrix_lcd);
  setMatrix("pin",     (uint32_t*)matrix_pin);
}




