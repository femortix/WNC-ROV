# WNC-ROV
<p><b>Community of Practice's code for a remote submersible. The idea is to allow it to traverse underwater locales and collect temperature, pressure, and possibly more, all autonomously.</b></p>
<p>• Programming is done with the Arduino IDE with an Arduino microcontroller. Arduino's language is most <i>similar</i> to C++, but not everything from C++ (notably the standard library) is native to Arduino, so keep that in mind.</p>
<p><a href="#section1">Preface</a></p>
<p><a href="#section2">Immediate To-Do</a></p>
<p><a href="#section3">Future To-Do</a></p>
<p><a href="#section4">Project Data</a></p>
<hr />
<h2 id="section1">Preface</h2><br />
<ul>
	<li>Potential changes or additions will only be discussed with actual CoP members.</li>
	<li>Feel free to download the repository, create a GitHub account, or both, to test the code base or learn with it in your spare time. It is also recommended you borrow Arduino-compatible devices and look up how to get it to function; solid for general programming familiarity.</li>
	<li>Previously all code was in the one .ino file present in the src directory. It has since been divided by hardware component into different .h files, united by the .ino file, so contributors can focus on more specific aspects of the project. It still works as intended despite this initial separation.</li>
</ul>
<hr />
<h2 id="section2">Immediate To-Do List</h2><br />
<ol>
	<li>Get the accelerometer and gyrometer workable.</li>
	<li>Mathematically keep track of position and orientation of the ROV</li>
</ol>
<hr />
<h2 id="section3">Future To-Do List</h2><br />
<ol>
	<li>Tie current and desired orientation (on XY-plane) to powering of motors</li>
	<li>Find and code a method to control buoyancy</li>
	<li>Tie current and desired depth to said buoyancy control</li>
</ol>
<hr />
<h2 id="section4">Project Data</h2><br />
<table>
	<tr>
		<th>Device</th>
		<th>Purpose</th>
	</tr>
	<tr>
		<td>Arduino Uno Rev3</td>
		<td>Microcontroller</td>
	</tr>
	<tr>
		<td>TB6612FNG</td>
		<td>Motor Controller</td>
	</tr>
	<tr>
		<td><i>???</i></td>
		<td>12V Waterproof Motors</td>
	</tr>
	<tr>
		<td>MPU-6050</td>
		<td>3.3V Accelerometer, Gyrometer</td>
	</tr>
	<tr>
		<td><i>???</i></td>
		<td>5V SD Card Reader</td>
	</tr>
	<tr>
		<td>DS18B20</td>
		<td>5V Waterproof Temperature Probe</td>
	</tr>
	<tr>
		<td>MPX4250A</td>
		<td>5V Waterproof (?) Pressure Sensor</td>
	</tr>
</table>
<p>Recorded Accelerometer/Gyrometer Errors: <a href="https://docs.google.com/spreadsheets/d/1X6XxIqIN3YmBvOW2JnSpvIH72xGgN9qytv67VfxdVCs/edit?usp=sharing">MPU-6050 Static Measurements (LSB)</a></p>
