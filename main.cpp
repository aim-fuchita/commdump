

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>


#define UART_DEVICE "/dev/rfcomm1"
//#define UART_DEVICE "/dev/serial/by-id/usb-USB_Vir_USB_Virtual_COM-if00"
#define UART_SPEED 115200

int uart_open(char const *device, int bps, struct termios *saveattr)
{
	int fd;
	int speed;
	struct termios attr;

	fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) {
		return -1;
	}

	tcgetattr(fd, &attr);
	*saveattr = attr;

	speed = B9600;
	switch (bps) {
	//	case      50:	speed =      B50;		break;
	//	case      75:	speed =      B75;		break;
	//	case     110:	speed =     B110;		break;
	//	case     134:	speed =     B134;		break;
	//	case     150:	speed =     B150;		break;
	// 	case     200:	speed =     B200;		break;
	case     300:	speed =     B300;		break;
	case     600:	speed =     B600;		break;
	case    1200:	speed =    B1200;		break;
		//	case    1800:	speed =    B1800;		break;
	case    2400:	speed =    B2400;		break;
	case    4800:	speed =    B4800;		break;
	case    9600:	speed =    B9600;		break;
	case   19200:	speed =   B19200;		break;
	case   38400:	speed =   B38400;		break;
	case   57600:	speed =   B57600;		break;
	case  115200:	speed =  B115200;		break;
	case  230400:	speed =  B230400;		break;
		//	case  460800:	speed =  B460800;		break;
		//	case  500000:	speed =  B500000;		break;
		//	case  576000:	speed =  B576000;		break;
		//	case  921600:	speed =  B921600;		break;
		//	case 1000000:	speed = B1000000;		break;
		//	case 1152000:	speed = B1152000;		break;
		//	case 1500000:	speed = B1500000;		break;
		//	case 2000000:	speed = B2000000;		break;
		//	case 2500000:	speed = B2500000;		break;
		//	case 3000000:	speed = B3000000;		break;
		//	case 3500000:	speed = B3500000;		break;
		//	case 4000000:	speed = B4000000;		break;
	}

	cfsetispeed(&attr, speed);
	cfsetospeed(&attr, speed);
	cfmakeraw(&attr);

	attr.c_cflag |= CS8 | CLOCAL | CREAD;
	attr.c_iflag = 0;
	attr.c_oflag = 0;
	attr.c_lflag = 0;
	attr.c_cc[VMIN] = 1;
	attr.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &attr);

	return fd;
}

void uart_close(int fd, struct termios *saveattr)
{
	tcsetattr(fd, TCSANOW, saveattr);
	close(fd);
}

class Dump {
private:
	unsigned int offset = 0;
	char buffer[100];
public:
	void put(char c)
	{
		int n = offset % 16;
		if (n == 0) {
			sprintf(buffer, "%08X", offset);
			memset(buffer + 8, ' ', 75 - 8);
			buffer[75] = 0;
		}
		char tmp[3];
		sprintf(tmp, "%02X", (unsigned char)c);
		buffer[10 + n * 3 + 0] = tmp[0];
		buffer[10 + n * 3 + 1] = tmp[1];
		buffer[10 + 16 * 3 + 1 + n] = (c >= 0x20 && c < 0x80) ? c : '.';
		offset++;
		if (offset % 16 == 0) {
			printf("\r%s\n", buffer);
		} else {
			printf("\r%s", buffer);
		}
		fflush(stdout);
	}
};

int main()
{
	Dump dump;

	fd_set readfds;
	int fd;
	struct termios stdinattr;
	struct termios uartattr;

	fd = uart_open(UART_DEVICE, UART_SPEED, &uartattr);
	if (fd < 0) return 1;

	// set to non canonical mode for stdin
	tcgetattr(0, &stdinattr);
	stdinattr.c_lflag &= ~ICANON;
	stdinattr.c_lflag &= ~ECHO;
	//	stdinattr.c_lflag &= ~ISIG;
	stdinattr.c_cc[VMIN] = 0;
	stdinattr.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &stdinattr);

	while (1) {
		char c;
		FD_ZERO(&readfds);
		FD_SET(0, &readfds); // stdin
		FD_SET(fd, &readfds);
		if (select(fd + 1, &readfds, 0, 0, 0) != 0) {
			if (FD_ISSET(0, &readfds)) {
				if (read(0, &c, 1) == 1) { // input from stdin
					if (c == '\n') {
						write(fd, "\r\n", 2);
					} else {
						write(fd, &c, 1);
					}
				}
			}
			if (FD_ISSET(fd, &readfds)) {
				if (read(fd, &c, 1) == 1) {
					dump.put(c);
				}
			}
		}
	}

	uart_close(fd, &uartattr);

	return 0;
}

