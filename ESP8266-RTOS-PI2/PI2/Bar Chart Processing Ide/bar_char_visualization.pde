import processing.serial.*;
import org.gicentre.utils.stat.*; 

BarChart barChart;
Serial myPort;
float array [];

void serialSetup() {
  //list all the ports
  printArray(Serial.list());
  //select the correct port : here it is the 32 port named : "/dev/ttyUSB0"
  myPort = new Serial(this, Serial.list()[32], 74880);
  //don't forget to change the baud rate to 115200
}

void canvaSetup() {


  barChart = new BarChart(this);
  barChart.setData(array);
//  barChart.getCo  lor.setColors("world", color(255,0,255) , color(255,0,0) );

  // Axis scaling
  barChart.setMinValue(0);
  barChart.setMaxValue(152);

  barChart.showValueAxis(true);
  barChart.showCategoryAxis(true);
  barChart.setBarColour(color(171,16,16));
  barChart.setBarGap(20);
  textFont(createFont("Ubuntu",30),20);
}

void setup() {
    size(1000, 400);
  //initialize the array that will contain all the values
  array = new float [] {0, 0, 0, 0, 0};
  //init canva
  canvaSetup();
  //init serial
  serialSetup();
}

void refreshData() {
  while (myPort.available() > 0) {
    String inBuffer = myPort.readStringUntil('\n'); 
    println(inBuffer);
    if (inBuffer != null) {
      //println(inBuffer);
      //split the string for every space
      String[]parts = inBuffer.split(" ");
      if (parts[0].equals("data")) {
        //show the different values
        for (int i = 1; i <= 5; i++) {
         // println(parts[i]);
        //update all the values
       // println(parts.length);
        array[i-1] = Float.parseFloat(parts[i]);
        }
        
        
      }
    }
  }
}

void draw() {
  background(15);
  refreshData();
  barChart.draw(15, 15, width-30, height-30); 
  
}