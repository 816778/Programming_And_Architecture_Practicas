In order to compile in Central, run:
g++ sobel_cl.cpp -o sobel -lOpenCL -I <path_to_cimg> -lm -lpthread -lX11 -ljpeg

After compiling, run the program as follows:
./sobel [-f|--file <image_file>] [-v|--verbose] [-w|--watch]

Options:
	-f|--file <image_file>
		The image to be loaded. Default: "images/image.jpg"
	-v|--verbose
		If the option is set, some information about the platform and the devices will be shown.
	-w|watch
		If the option is set, image before and after Sobel will be displayed on the screen.