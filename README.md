# Autonomous Survey Submersible
<p><b>Small submersible equipped with sensors to read local temperature, pressure, accelerations, and angular velocities.</b></p>
<ul>
	<li>Community of Practice's STEM-related project from Fall 2023 - Spring 2024.</li>
	<li>There is an educational motif present throughout the code base.</li>
	<li>If a current CoP group would like to change and use this code base themselves, feel free to let me know.</li>
</ul>
<details>
	<summary>
		Version History
	</summary>
	<p><b>Version 0.1.0</b> (11/2023) - T. W. Herring<br />
	- Code to control motors, read temperature and pressure data, and read/write to SD card.<br />
	- Code to calculate depth based on pressure; logs sensor data to a .csv.</p>
	<p><b>Version 0.1.1</b> (02/2024) - Damian Contreras<br />
	- Pin numbers #define'd, maximum file count raised to 9999, each component separated into their own .h file.<br />
	- Pressure calculation slightly modified based on sensor documentation voltages.</p>
	<p><b>Version 0.2.0</b> (04/2024) - Damian Contreras<br />
	- Physical class added to hold position, velocity, acceleration, angle, and angular velocity, and update all of them.<br />
	- MPU-6050 functionality added; pitching submersible vertically toggles it on/off in a test loop.</p>
</details>
<p><a href="#s1">Preface</a></p>
<p><a href="#s2">Project Data</a></p>
<p><a href="#s3">Issues</a></p>
<p><a href="#s4">Future Considerations</a></p>
<hr />
<h2 id="s1">Preface</h2><br />
<ul>
	<li>Many files call this autonomous submersible a Remote Operated Vehicle. As it turns out, communicating through even a meter of water is difficult, but "ROV" stuck.</li>
	<li>Programming is done with the Arduino IDE and an Arduino microcontroller. Arduino's language is most <i>similar</i> to C++, but not everything from C++ (notably the standard library) is native to Arduino, so keep that in mind.</li>
	<li>Feel free to download the repository to test the code base or learn with it in your spare time. It is also recommended you borrow Arduino-compatible devices and look up how to get them to function; solid for general programming familiarity.</li>
</ul>
<hr />
<h2 id="s2">Project Info</h2><br />
<table>
	<tr>
		<th>Device</th>
		<th>Purpose</th>
		<th>Links</th>
	</tr>
	<tr>
		<td>Arduino Uno Rev3</td>
		<td>Microcontroller</td>
		<td><a href="https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf">Datasheet</a></td>
	</tr>
	<tr>
		<td>TB6612FNG</td>
		<td>Motor Controller</td>
		<td><a href="https://www.sparkfun.com/datasheets/Robotics/TB6612FNG.pdf">Datasheet</a></td>
	</tr>
	<tr>
		<td>???</td>
		<td>12V Waterproof Motors (x2)</td>
		<td><a></a></td>
	</tr>
	<tr>
		<td>MPU-6050</td>
		<td>3.3V Accelerometer, Gyrometer</td>
		<td><a href="https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf">Datasheet</a><br />
		<a href="https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf">Registers</a></td>
	</tr>
	<tr>
		<td>MicroSD Breakout board+</td>
		<td>5V SD Card Reader</td>
		<td><a href="https://cdn-learn.adafruit.com/downloads/pdf/adafruit-micro-sd-breakout-board-card-tutorial.pdf">Tutorial</a></td>
	</tr>
	<tr>
		<td>DS18B20</td>
		<td>5V Waterproof Temperature Probe</td>
		<td><a href="https://cdn-shop.adafruit.com/datasheets/DS18B20.pdf">Datasheet</a></td>
	</tr>
	<tr>
		<td><s>MPX4250A</s></td>
		<td>5V Pressure Sensor (Old)</td>
		<td><a href="https://www.nxp.com/docs/en/data-sheet/MPX4250A.pdf">Datasheet</a></td>
	</tr>
	<tr>
		<td>???</td>
		<td>5V Waterproof Pressure Sensor</td>
		<td><a href="https://www.amazon.com/Pressure-Sensor-Connection-Analog-Accuracy/dp/B0C2NG268Q/">Store</a></td>
	</tr>
</table>
<h2 id="s3">Issues</h2><br />
<ul>
	<li>The MPU-6050's acceleration is now exclusively returning 1 instead of something over its full range [-32768, 32767].</li>
	<li>Extremely tiny changes to mass distribution cause leaning, affecting movement.</li>
	<li>The wires are shockingly loose; ideally they should be soldered when you're confident in your setup.</li>
	<li>The pressure-sensing code only applies to the MPX4250A (and even then it may be off), not the bulky waterproof guy. Gather some experimental data like was done with the MPU-6050.</li>
	<li>Having to call a function twice to access a register (unless that's intended in the library, not sure).</li>
</ul>
<h2 id="s4">Considerations</h2><br />
<ol>
	<li>Buoyancy control, or another vertical movement method? Below consideration is relevant to your chosen method.</li>
	<li>Either way, will you need to upscale to an Arduino Mega? You will if you add another motor or servo or digital pin-heavy chip.</li>
	<li>Depending on that, and any additional sensors, what do you want to use as the hull? Unsure whether hot glue for plastic holes or resin for resin holes is better.</li>
	<li>Do you have enough people to delegate into specific enough roles? Speaking from experience, try to have "task forces" (I didn't push that agenda, but maybe should have).</li>
</ol>
