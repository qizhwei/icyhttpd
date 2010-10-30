.PHONY: clean All

All:
	@echo ----------Building project:[ icyhttpd - Debug ]----------
	@"mingw32-make.exe"  -j 2 -f "icyhttpd.mk"
clean:
	@echo ----------Cleaning project:[ icyhttpd - Debug ]----------
	@"mingw32-make.exe"  -j 2 -f "icyhttpd.mk" clean
