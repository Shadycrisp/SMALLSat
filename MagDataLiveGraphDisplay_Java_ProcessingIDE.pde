import processing.serial.*;

Serial myPort; 
String serialData; 
ArrayList<Float> xValues = new ArrayList<Float>(); 
ArrayList<Float> yValues = new ArrayList<Float>(); 
ArrayList<Float> zValues = new ArrayList<Float>(); 
int graphX = 80; 
int maxDataPoints = 500; 
float graphWidth = 500; 
float minY = -2000, maxY = 2000; 

void setup() {
  size(600, 600);
  println("Available serial ports:");
  printArray(Serial.list());
  myPort = new Serial(this, Serial.list()[0], 115200);
  myPort.bufferUntil('\n'); // Read up to the newline character
}

void draw() {
  background(255); // Set background to white
  drawGraph();
  drawAxes();
}

void serialEvent(Serial myPort) {
  serialData = myPort.readStringUntil('\n');
  serialData = trim(serialData);
             //Data from arudino has to be set the same, "\n" between each readings, and "MAG,x,y,z" for magnetometer output
  if (serialData.startsWith("MAG,")) {
    String[] magData = split(serialData.substring(4), ',');
    if (magData.length == 4) { 
   
      float x = float(magData[1]);
      float y = float(magData[2]);
      float z = float(magData[3]);
      

      xValues.add(x);
      yValues.add(y);
      zValues.add(z);
      
 
      if (xValues.size() > maxDataPoints) {
        xValues.remove(0);
        yValues.remove(0);
        zValues.remove(0);
      }
    }
  }
}

void drawGraph() {
  // X values in red
  stroke(255, 0, 0); // Red
  drawSingleAxisGraph(xValues);

  // Y values in green
  stroke(0, 255, 0); // Green
  drawSingleAxisGraph(yValues);

  // Z values in blue
  stroke(0, 0, 255); // Blue
  drawSingleAxisGraph(zValues);
}

void drawSingleAxisGraph(ArrayList<Float> values) {
  noFill();
  beginShape();
  for (int i = 0; i < values.size(); i++) {
    float x = map(i, 0, values.size(), graphX, graphX + graphWidth);
    float y = map(values.get(i), minY, maxY, height-100, 100); // Map based on Y-axis range
    vertex(x, y);
  }
  endShape();
}

void drawAxes() {
  stroke(0);
  // Y axis
  line(graphX, 50, graphX, height-50);
  // X axis
  line(graphX, height-100, graphX + graphWidth + 20, height-100);

  //Axis Lables
  textSize(12);
  fill(0);
  text("Magnetometer Reading (nT)", graphX - 75, 40);
  text("Time (arbitrary units)", graphX + graphWidth - 100, height - 70);
  

  int numOfMarkers = 5; // How many markers we want on the Y-axis
  for (int i = 0; i <= numOfMarkers; i++) {
    float y = map(i, 0, numOfMarkers, height-100, 100);
    line(graphX - 5, y, graphX + 5, y); // Small tick for scale marker
    
    // Label for the scale marker
    float labelValue = map(i, 0, numOfMarkers, minY, maxY);
    text(nf(labelValue, 0, 2), graphX - 75, y + 5);
  }
}
