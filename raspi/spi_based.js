// Stand alone example of SPI based sonar interface.

const SPI = require('pi-spi');
const hexToBinary = require('hex-to-binary');

class spiInterface {
  constructor() {
    this.name = 'SPI';
    this.sonarInterval = 300, // in ms
    this.hasSPI = true;
    try {
      this.spi = SPI.initialize('/dev/spidev0.1');
      this.spi.clockSpeed(25000);
    } catch (err) {
      logger.error(`[${this.name}] ${err.message}`);
      this.hasSPI = false;
    }
    this.sonarHeader = '11111111111111111111111111111100000000000000';
    this.sonarRe = /0+/;

    // Launch the sonar data read loop
    if (!this.hasSPI) return;
    setInterval(() => {
      this.sonarPromise().then((result) => {
        this.parseSonarMsg(result);
      }, (err) => {
        logger.error(`[${this.name}] ${err.message}`);
      });
    }, this.sonarInterval);
  }

  sonarPromise() {
    return new Promise((resolve, reject) => {
      this.spi.read(200, (err, data) => {
        if (err) {
          reject(err);
        } else {
          resolve(hexToBinary(data.toString('hex')));
        }
      });
    });
  }

  parseSonarMsg(msg) {
    // Get just one message out of the data
    const sMsg = msg.substring(msg.indexOf(this.sonarHeader), msg.indexOf(this.sonarHeader) + 320);
    // Split string of ones and zeros to an array of ones
    const onesArray = sMsg.split(this.sonarRe);
    let snrA = '';
    let snrB = '';
    let snrC = '';
    let snrD = '';
    // TODO: Clean this up.
    onesArray.slice(2, 10).forEach((ones) => {
      if (ones === '11111' || ones === '111111') snrA += '1';
      else snrA += '0';
    });
    onesArray.slice(10, 18).forEach((ones) => {
      if (ones === '11111' || ones === '111111') snrD += '1';
      else snrD += '0';
    });
    onesArray.slice(18, 26).forEach((ones) => {
      if (ones === '11111' || ones === '111111') snrC += '1';
      else snrC += '0';
    });
    onesArray.slice(26, 34).forEach((ones) => {
      if (ones === '11111' || ones === '111111') snrB += '1';
      else snrB += '0';
    });
    const sensorData = {
      sensor: 'sonar',
      data: {
        left: parseInt(snrA, 2),
        center: parseInt(snrB, 2),
        right: parseInt(snrC, 2),
        rear: parseInt(snrD, 2),
      },
    };
    console.log(sensorData);
  }
};


// Instantiate an SPI based sonar interface
var sonar = new spiInterface();
