
<body>

  <h1>CSE-331: Hardware Practice and Test</h1>
  <p>
    This repository contains <strong>hardware practice experiments</strong> and <strong>test preparations</strong> for the CSE-331 course. 
    It covers hardware requirements, software tools, and practice/test tasks to help build hands-on embedded systems skills.
  </p>

  <hr />

  <h2>🖥️ Software Requirements</h2>
  <ol>
    <li><strong>STM32CubeMX</strong> — peripheral configuration and code generation.</li>
    <li><strong>Keil uVision5</strong> — compile and debug STM32 code.</li>
    <li><strong>STM32 ST-LINK Utility</strong> — flash/debug STM32 via ST-LINK.</li>
  </ol>

  <h2>🔧 Hardware Requirements</h2>
  <table>
    <thead>
      <tr>
        <th>#</th>
        <th>Hardware Name</th>
        <th>Type</th>
        <th>Required</th>
        <th>Example Link</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>1</td>
        <td>STM32F103C8T6 — Blue Pill</td>
        <td>MCU</td>
        <td><span class="badge">Must</span></td>
        <td><a href="https://store.roboticsbd.com/arduino-bangladesh/350-stm32f103c8t6-blue-pill-development-board-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>2</td>
        <td>ST-LINK V2 in-circuit debugger/programmer</td>
        <td>Programmer</td>
        <td><span class="badge">Must</span></td>
        <td><a href="https://store.roboticsbd.com/programmer/1102-st-link-stlink-st-link-v2-mini-stm8-stm32-simulator-download-programmer-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>3</td>
        <td>16×2 LCD (I²C) <em>or</em> 0.96″ OLED (I²C)</td>
        <td>Display (Peripheral)</td>
        <td><span class="badge">Must</span></td>
        <td>
          <a href="https://store.roboticsbd.com/arduino-shield-module/1349-16x2-serial-lcd-module-display-for-arduino-assembled-robotics-bangladesh.html">16×2 LCD</a> /
          <a href="https://store.roboticsbd.com/arduino-shield-module/1328-096-inch-i2c-oled-display-white-robotics-bangladesh.html">0.96″ OLED</a>
        </td>
      </tr>
      <tr>
        <td>4</td>
        <td>4×4 Keypad Matrix</td>
        <td>Input Peripheral</td>
        <td><span class="badge">Must</span></td>
        <td><a href="https://store.roboticsbd.com/arduino-shield-module/986-4x4-keypad-16-key-matrix-membrane-type-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>5</td>
        <td>DHT11 Temperature &amp; Humidity Sensor</td>
        <td>Sensor</td>
        <td><span class="badge">Must</span></td>
        <td><a href="https://store.roboticsbd.com/sensors/2654-dht11-temperature-and-humidity-sensor-module-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>6</td>
        <td>Ultrasonic Sonar Sensor (HC-SR04)</td>
        <td>Sensor</td>
        <td><span class="badge">Must</span></td>
        <td><a href="https://store.roboticsbd.com/sensors/22-ultrasonic-sonar-sensor-hc-sr04-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>7</td>
        <td>Servo Motor (SG90, 360° continuous)</td>
        <td>Actuator</td>
        <td><span class="badge">Must</span></td>
        <td><a href="https://store.roboticsbd.com/motor/2450-servo-motor-micro-sg90-360-degree-continuous-rotation-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>8</td>
        <td>IR Sensor</td>
        <td>Sensor</td>
        <td><span class="badge badge-optional">Optional</span></td>
        <td><a href="https://store.roboticsbd.com/sensors/685-infrared-obstacle-avoidance-ir-sensor-module-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>9</td>
        <td>Soil Moisture Sensor (YL-69)</td>
        <td>Sensor</td>
        <td><span class="badge badge-optional">Optional</span></td>
        <td><a href="https://store.roboticsbd.com/sensors/145-yl-69-soil-hygrometer-humidity-soil-moisture-detection-sensor-robotics-bangladesh.html">RoboticsBD</a></td>
      </tr>
      <tr>
        <td>10</td>
        <td>Any other modules (e.g., GPS, accelerometer, GSM, voice/fire/pollution detection)</td>
        <td>Sensor/Peripheral</td>
        <td><span class="badge badge-optional">Optional</span></td>
        <td><span class="muted">Examples listed as options</span></td>
      </tr>
    </tbody>
  </table>

  <p class="footnote">
    Source: Hardware Requirements PDF :contentReference[oaicite:0]{index=0}
  </p>

  <h2>📝 Practice &amp; Test Progress</h2>
  <ul class="checklist">
    <li>✅ Practice 1 — Basic STM32 GPIO (LED blinking)</li>
    <li>⬜ Practice 2 — Interfacing 16×2 LCD / OLED</li>
  </ul>
  <p class="muted">I will keep adding completed practices and test results here.</p>

  <h2>🚀 How to Use</h2>
  <ol>
    <li>Clone the repository:
      <pre><code>git clone https://github.com/Rokib-Hasan-Oli/CSE-331-Hardware-Practice-And-Test.git
cd CSE-331-Hardware-Practice-And-Test</code></pre>
    </li>
    <li>Configure peripherals with <strong>STM32CubeMX</strong>.</li>
    <li>Open project files in <strong>Keil uVision5</strong>.</li>
    <li>Flash/debug the MCU using <strong>STM32 ST-LINK Utility</strong>.</li>
  </ol>

  <h2>📖 License</h2>
  <p>
    This repository is for <strong>educational purposes</strong>. You are free to use, modify, and share the code.
  </p>

</body>
</html>

