void square(int a) {
	int b = a * a;
	System.out.println(b);
}

int main() {
	int a = 23;
	int e = 2;
	int b = a;
	int c = a * 2;
	b = c * a;
	if(a > b) {
		c = 30;
	}
	else {
		c = 15;
	}
	System.out.println(c*2);
	square(c);
}