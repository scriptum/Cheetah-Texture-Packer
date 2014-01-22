Cheetah Texture Packer
------------------------------

Just best texture packer. It uses the idea of MaxRects bin packing algorithm with aggressive heuristics.

This tool was created as research implementation and never been released. But it works and you can compile this tool for any platform you want.

If you know Russian - welcome: http://www.gamedev.ru/projects/forum/?id=161714

Watch me if you want to see changes :-)

This program uses UBFG's algorithm: https://github.com/scriptum/UBFG

Format of .atlas file (same as UBFG's .fnt):

	textures: atlas_image.png --image name with packed textures
	Name     X pos  Y pos   Width  Height   Xoffset  Yoffset  Orig W  Orig H   Rot
	img1     0      0       0      0        3        14       3       14
	img2     90     36      5      7        1        4        7       14       r
	dir/img  0      41      5      9        1        2        7       14


Here:

* Name - name of packed image with full path
* X pos - x position of image on texture
* Y pos - y position of image on texture
* Width - width of image on texture (images are cropped and Width and Orig Width aren't equal)
* Height - height of image on texture
* Xoffset - distance on the x-axis, on which image must be shifted
* Yoffset - distance on the y-axis, on which image must be shifted
* Orig W - original width of image
* Orig H - original height of image
* Rotation - if "r" - image rotated

Additional command line features

```
--disable-merge //do not merge similar images
--disable-crop //do not crop images
--disable-recursion //just one folder not subfolder
--disable-border //do not make border
--enable-rotate //enable sprites rotation
--square //only square atlases
--extrude-size value //size of extrude image in pixels
--border-size value //border size in pixels
--autosize-threshold value //use autosize algorithm only if fill rate in percents < value
--min-texture-size WidthxHeight //use autosize algorithm only if last texture size > min texture size
--crop-threshold value //crop alpha threshold 0-255
--sortorder value //sort order line in interface new sort order algorithm MAX
```

![Help image](https://github.com/scriptum/UBFG/raw/master/readme.png)

Screenshot:

![Screenshot](https://github.com/scriptum/Cheetah-Texture-Packer/raw/master/screen.png)

References:

* http://clb.demon.fi/files/RectangleBinPack.pdf
* http://clb.demon.fi/projects/more-rectangle-bin-packing
