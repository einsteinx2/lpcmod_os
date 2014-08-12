#include "boot.h"
#include "BootFlash.h"
#include "memory_layout.h"

void DisplayProgressBar(int currentVal, int maxVal, unsigned long color) {
	int x,y,l,w,h,m;
	u32 *fb=(u32*)FB_START;

	if(maxVal<2){
		return;
	}

	w=vmode.width;
	h=vmode.height;
	l=w-100;
	y=h-100;
	m=((w-150)>>2)*currentVal/maxVal;
	m*=4;
	m+=50;
	for(x=50;x<l;x++){
		fb[y*w+x]=0xffffffff;
		fb[(y+1)*w+x]=0xffffffff;
		if(x>55 && x<m){
			int z;
			for(z=5;z<45;z++){
				fb[(y+z)*w+x]=color;
			}
		}
		fb[(y+50)*w+x]=0xffffffff;
		fb[(y+51)*w+x]=0xffffffff;
	}
	for(;y<h-50;y++){
		fb[y*w+51]=0xffffffff;
		fb[y*w+50]=0xffffffff;
		fb[y*w+l-1]=0xffffffff;
		fb[y*w+l-2]=0xffffffff;
	}
}

