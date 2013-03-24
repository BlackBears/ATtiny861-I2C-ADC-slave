#### About the project ####

I wrote this code to demonstrate the I2C slave functionality of ATtiny x61 series of MCU's, such as the [ATtiny861](), the [ATtiny461](), and the [ATtiny261]().  So far, I've only tested it on the ATtiny861 because that's what I have available.  The code is quite small; so it would be a better fit for a smaller memory device.

#### Context ###

The project is meant to support the missing ADC functionality on the Raspberry Pi. The RPi has no on-board ADC. To achieve ADC capabilities, we could use a discrete ADC chip; but most of them use a custom SPI protocol that requires a lot of bit-banging. I’d rather just write an I2C-based ADC on AVR than deal with all of that on the RPi.

#### Requirements ####

To use this code, you will need the appropriate hardware and an environment for compiling and uploading the code to the MCU.  These requirements include:

- An **I2C master** device somewhere on the bus.  _In this case, I'm using the RPi as the I2C master._
- An ATtiny x61 MCU or a device similar to it.
- [AVR Studio 6](http://www.atmel.com/microsite/atmel_studio6/)

#### Note ####

You should either run the MCU at 3V3 or use some sort of logic level conversion if you really want to run at 5V. The RPi is 3V3 as are the other devices on its I2C bus; and if you have a 5V slave on the bus, you risk damaging both the RPi and the other slaves. This is my favorite way of doing bidirectional logic level conversion:

![MOSFET logic level conversion](http://www.hobbytronics.co.uk/image/data/tutorial/mosfet_level_converter.jpg)

You can also use some pre-built solutions for logical level conversion, e.g. break-out boards.

#### Theory of operation ####

The MCU functions as an I2C slave device and requires an I2C master on the bus.  The purpose of the slave is to sample one of the several ADC's and return its 10 bit value to the master.  The target MCU for this code uses a Universal Serial Interface (USI) for I2C-compatible TWI functionality.  To realize this capability in software, we used Dan Gates' adaptation of the operation described in [AVR312](http://www.atmel.com/Images/doc2560.pdf): _Using the USI module as a I2C slave._  A complete description of the I2C protocol is beyond the scope of this article; but the application note summarizes the master-slave interaction as follows:

> A START condition is always followed by the (unique) 7-bit slave address and then by a Data Direction bit. The Slave device addressed now acknowledges to the Master by holding SDA low for one clock cycle. If the Master does not receive any acknowledge the transfer is terminated. Depending of the Data Direction bit, the Master or Slave now transmits 8-bit of data on the SDA line. The receiving device then acknowledges the data. Multiple bytes can be transferred in one direction before a repeated START or a STOP condition is issued by the Master. The transfer is terminated when the Master issues a STOP condition. A STOP condition is defined by a low to high transition on the SDA line while the SCL is high.

> If a Slave device cannot handle incoming data until it has performed some other function, it can hold SCL low to force the Master into a wait-state.

> All data packets transmitted on the TWI bus are 9 bits long, consisting of one data byte and an acknowledge bit. During a data transfer, the master generates the clock and the START and STOP conditions, while the receiver is responsible for acknowledging the reception. An Acknowledge (ACK) is signaled by the receiver pulling the SDA line low during the ninth SCL cycle. If the receiver leaves the SDA line high, a NACK is signaled.

The USI on the ATting861 has multiple modes of operation.  As described in the application note referenced above, we are making use of this interface as an I2C slave.  This driver is implemented in `usiTwiSlave.c` and is included in the project.

The master should read two bytes from the desired ADC.

#### Using the system with Raspberry Pi ####

You will need to compile the code in AVR Studio 6 or whatever similar tool you use.  _(I like AVR Studio 6.  There are all sorts of arguments on forums like [AVR Freaks](http://www.avrfreaks.net) about what the best environment is.  It's mostly an opinion based thing.)_  You will need to load it onto the MCU.  How to do that is beyond the scope of these instructions, but some of the tools that I use are:  [AVRISP mkII](http://www.atmel.com/tools/AVRISPMKII.aspx), and [AVR programming adapter](https://www.sparkfun.com/products/8508) from Sparkfun.

On the Raspberry Pi side, you need to set up I2C functionality of the device.  Basically, it's just:

    sudo apt-get install python-smbus
    sudo apt-get install i2c-tools

You will also need to edit `/etc/modules` to include the following lines:

	i2c-dev
	i2c-bcm2708

For the hardware, I use the [Adafruit Pi Cobbler](https://www.adafruit.com/products/914) to make it easier to prototype with the RPi.  Once you have everything connected you can check if the device is on the bus with:

    sudo i2cdetect -y 1

or, if you have the original Raspberry Pi, it's

	sudo i2cdetect -y 0

This will print a grid to the console; and you should see device `26` in the grid.

![i2cdetect results](http://i.imgur.com/K8q3vKl.png)

Now, you can read a value from an ADC:

    sudo i2cget -y 1 0x26 0x00 w

 ![i2cget results](http://i.imgur.com/YSV9331.png)

 #### Reading the on-chip temperature ####

 The ATtinyx61 series permits reading the on-chip temperature by accessing a special internal ADC channel.

 You can also read the on-chip temperature by reading from register `0x3F`:

 	sudo i2cget -y 1 0x26 0x3F w

You will need to convert the returned ADC value to degrees C via a linear formula based on calibration.  According to the datasheet:

> The measured voltage has a linear relationship to the temperature as described in Table 15-2.  The sensitivity is approximately 1 LSB/°C and the accuracy depends on the method of user calibration.  Typically, the measurement accuracy after a single temperature calibration is ±10°C, assuming calibration at room temperature.  Better accuracies are achieved by using two temperature points for calibration. 

I recommend using two different temperatures and just use linear data modeling _(e.g. `y = mx+b`)_ to calibrate the following equation `T (°C) = k * [ (ADCH << 8) | ADCL ] + Tos`.  Where `k` is the fixed slope coefficient and `Tos` is the temperature sensor offset.  The datasheet says it is close to 1.0 typically.


 