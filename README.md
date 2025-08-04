# Templates_Arduino: Arduino Master Template 

## Master Setup 

```
void setup() 
{
  setup_serial();
  Serial.println("Exhibit /habitat/luft: starting setup ------------------------------------------------------------");
  
  setup_matrix();
  show_matrix("init", 2000);

  show_matrix("lcd", 2000);
  setup_lcd();

  show_matrix("pin", 2000);
  setup_pins();

  show_matrix("clock1", 2000);
  wlan_delay(60); // wait till unifi dreambox is also ready
  show_matrix("net", 1000);
  setup_multiwlan(); // with small delay for IP

  show_matrix("mqtt", 2000);
  setup_mqtt(); // unique client name 
  
  Serial.println("Exhibit /habitat/luft: setup complete ------------------------------------------------------------");
}
```

## Serial Setup

With general Infos from Compiler.
```
void setup_serial() {
  Serial.begin(BAUDRATE); // or slow: 9600
  while (!Serial) { }; // wait for serial port to connect. Needed for native USB port only 
  delay(200);
  Serial.println("Exhibit /habitat/luft starting...");
 
  Serial.println("");
  Serial.print(F("Starting UP File: "));
  Serial.println(__FILE__);
  Serial.print(F("code creation [NIS]: "));
  Serial.println(": " __DATE__ " @ " __TIME__);  // predefined macors
  Serial.println(F("--------------------------------------------------------------------------------"));
}
```

## Matrix.h 

Helper for Arduino R4 Matrix display. Different Symbols in Matrix.h can be used like
*  show_matrix("init", 2000);

```
void setup_matrix()
{
  matrix.begin();
  setup_symbols(); // in matrix.h
}

void show_matrix(String matrix_name,int time)
{
  matrix.loadFrame(getMatrix(matrix_name));
  delay(time);
}
```


