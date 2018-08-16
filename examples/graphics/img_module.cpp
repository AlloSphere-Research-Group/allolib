#include "../modules/img/loadImage.hpp"

#include <iostream>

using namespace std;

int main () {
	auto imageData = img_module::loadImage("data/img.jpeg");
	if (imageData.data.size() == 0) {
		cout << "failed to load image" << endl;
		return 1;
	}
	cout << imageData.width << ", " << imageData.height << endl;
	cout << imageData.data.size() << endl;
	unsigned char r, g, b;
	float brightness;
	int print_char;
	for (size_t i = 0; i < imageData.height; i += 1) {
		for (size_t j = 0; j < imageData.width; j += 1) {
			r = imageData.data[0 + j * 4 + i * imageData.width * 4];
			g = imageData.data[1 + j * 4 + i * imageData.width * 4];
			b = imageData.data[2 + j * 4 + i * imageData.width * 4];
			// auto a = imageData.data[3 + j * 4 + i * imageData.width * 4];
			brightness = (r + g + b) / 3.0f / 255.0f;
			print_char = int(brightness * 9);
			cout << print_char;
		}
		cout << endl;
	}
}
