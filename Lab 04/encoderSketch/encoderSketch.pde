import processing.serial.*;

Serial port;
PrintWriter out;

void setup() {
  println(Serial.list());                 
  String portName = Serial.list()[3];     // <-- change to your port index
  port = new Serial(this, portName, 115200);
  port.bufferUntil('\n');

  String fn = nf(year(),4)+nf(month(),2)+nf(day(),2)+"_"+nf(hour(),2)+nf(minute(),2)+nf(second(),2);
  out = createWriter("arduino_log_"+fn+".csv");
  out.println("t_ms,d1,d2");
}

void serialEvent(Serial p) {
  String line = trim(p.readStringUntil('\n'));
  if (line == null || line.length() == 0) return;

  String[] parts = split(line, ',');
  if (parts.length == 2) {
    int d1 = int(parts[0]);
    int d2 = int(parts[1]);
    out.println(millis()+","+d1+","+d2);
    out.flush();   // <--- force write to disk immediately
  }
}

void keyPressed() { out.flush(); out.close(); exit(); }
void draw() {
}
