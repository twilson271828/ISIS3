APPNAME = rgb2hsv

rgb2hsvTruth4.cub.TOLERANCE = 0.02
rgb2hsvTruth5.cub.TOLERANCE = 0.0001
rgb2hsvTruth6.cub.TOLERANCE = 0.0001

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) red=$(INPUT)/isisTruth.cub+5 \
	green=$(INPUT)/isisTruth.cub+4 \
	blue=$(INPUT)/isisTruth.cub+3 \
	hue=$(OUTPUT)/rgb2hsvTruth4.cub \
	saturation=$(OUTPUT)/rgb2hsvTruth5.cub \
	value=$(OUTPUT)/rgb2hsvTruth6.cub \
	mode=manual \
	rmin=10 \
	rmax=250 \
	gmin=10 \
	gmax=250 \
	bmin=10 \
	bmax=250 > /dev/null;
