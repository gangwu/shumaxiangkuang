#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<linux/fb.h>
#include<sys/mman.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<jpeglib.h>
#include<jerror.h>
#include<stdlib.h>

unsigned short  RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue);


int main(int argc,char *argv[])
{
 int fbfd=0;
 int fb=0;
 struct fb_var_screeninfo vinfo;
 struct fb_fix_screeninfo finfo;
 long int screensize = 0;
 char *fbp = 0;
 struct jpeg_decompress_struct cinfo;
 struct jpeg_error_mgr jerr;
 int x,y;
/*以读写方式打开显示设备节点文件/dev/fb0*/
 fbfd=open("/dev/fb0",O_RDWR);
 if(fbfd<=0){
 printf("error:can not open framebuffer device.\n");
 return -1;
 }
 printf("the framebuffer device was opened successfully.\n");
/*得到framebuffer设备的固定屏幕信息
*struct fb_fix_screeninfo 结构包含主要的成员：
* unsigned long seme_start;描述缓冲区的起始地址（物理地址）
*__u32  smem_len;描述缓冲取的长度；
*——u32  type;描述fb类型，比如TFT或STN类型
*__u32  visual;描述显示颜色是真彩色，伪彩色，还是单色
*该结构用来描述与设备无关，不可变更的信息。用户可以使用FBIOGET—FSCREENINFO命令来获得这些信息
*方法如下：
*/
//本程序没有用到固定参数所以如下：
// if(ioctl(fbfd,FBIOGET_FSCREENINFO,&finfo)){
//  printf("error reading fixed information.\n");
//  return -2;
// }
/*得到可变屏幕信息*/
/*_
*fb_var_screeninfo 结构成员如下：
*__u32 xres;     //可见分辨率xres*yres 如1024*768、800*600等即为传说的像素，也可看成显示屏的
*__u32 yres;      x 轴 y 轴。
*__u32 xres_virtual;//虚拟分辨率
*__u32 yres_virtual;
*__u32 xoffset;//从虚拟到可见分辨率的偏移
*__u32 yoffset;
*以及屏幕四周的 margin（边距），像素时钟，同步时序等宝贵信息。
*该结构描述设备无关的，可更改的配置信息。应用程序可以使用FBIOGET—VSCREENINFO命令获得这些信息
*方法如下：
*使用FBIOPUT_VSCREENINFO命令写入这些信息。（本人没试过）
*/
 if(ioctl(fbfd,FBIOGET_VSCREENINFO,&vinfo)){
  printf("error reading variable information.\n");
  return -3;
 }
/*将映射的显存的大小*/ 
//bits_per_pixle  多少位色（32位、24位、16位，每个像素位色）
 screensize = vinfo.xres*vinfo.yres*vinfo.bits_per_pixel/8;
/*将显存映射用户空间*/
 fbp=(char*)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fbfd,0);
 if((int)fbp==-1){
 printf("error:failed to map framebuffer device to memory.\n");
 return -4;
 }
 printf("the framebuffer device was mapped to memory successfully.\n");
 
 
 FILE *infp;
 unsigned char *buffer;
/*   打开要显示的图片*/
 if ((infp=fopen(argv[1],"r"))==NULL)
 {
  printf("Error:open %s failed\n",argv[1]);
  exit(0);
 } 
/*读取图片参数*/
/*
*解压缩过程中使用的JPEG对象是一个jpeg_decompress_struct的结构体。同时还需要定义一个用于错误处理的结构体对象，IJG中标准的错误结构体是jpeg_error_mgr。
*struct jpeg_decompress_struct cinfo; 
*struct jpeg_error_mgr jerr;
*然后是将错误处理结构对象绑定在JPEG对象上。
*cinfo.err = jpeg_std_error(&jerr);
*这个标准的错误处理结构将使程序在出现错误时调用exit()退出程序，如果不希望使用标准的错误处理方式，则可以通过自定义退出函数的方法自定义错误处理结构
*/
 cinfo.err=jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);//初始化cinfo结构
/*利用标准C中的文件指针传递要打开的jpg文件。
* FILE * infile;
*if ((infile = fopen("sample.jpg", "rb")) == NULL) { return 0; }
* jpeg_stdio_src(&cinfo, infile);  
*/
    jpeg_stdio_src(&cinfo, infp);
/*将图像的缺省信息填充到cinfo结构中以便程序使用。
  (void) jpeg_read_header(&cinfo, TRUE);  
*此时，常见的可用信息包括图像的宽cinfo.image_width，高cinfo.image_height，色彩空间cinfo.jpeg_color_space，颜色通道数cinfo.num_components等。
*/
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);//开始解压缩在完成解压缩操作后，IJG就会将解压后的图像信息填充至cinfo结构中。比如，输出图像宽度cinfo.output_width，输出图像高度cinfo.output_height，每个像素中的颜色通道数cinfo.output_components（比如灰度为1，全彩色为3）等。
/*output_width图片实际宽，output_height 图片实际高度*/
 if ((cinfo.output_width > vinfo.xres) || (cinfo.output_height > vinfo.yres))
  {
   printf("too large JPEG file,cannot display\n");
   return (-1);
     }
/*output_components图片总行数*每行的output_width等于图片的大小*/
    buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_components);
 y=0;
/*画图*/
  while (cinfo.output_scanline < cinfo.output_height) 
 {
         jpeg_read_scanlines(&cinfo, &buffer, 1);//扫描行
         if (vinfo.bits_per_pixel== 16) //如果显示设备时16位色，则需要把采集的24位色转换成16位色。
   {
          unsigned short  color;//每像素的RGB
             for (x=0; x < cinfo.output_width; x++)
     {
      color = RGB888toRGB565(buffer[x * 3], buffer[x * 3 + 1], buffer[x * 3 + 2]);
      if ((x > vinfo.xres) || (y > vinfo.yres)) 
                return (-1);
               unsigned short *dst = ((unsigned short *) fbp + y * vinfo.xres + x);
                 *dst = color;      
    }  
           
         } else if (vinfo.bits_per_pixel == 24)
    {
      memcpy((unsigned char *)fbp  + y * vinfo.xres * 3, buffer, cinfo.output_width * cinfo.output_components);
           }
             y++;
         }
// jpeg_finish_decompress(&cinfo);
// jpeg_destory_decompress(&cinfo);
 free(buffer);
 fclose(infp);
 munmap(fbp,screensize);
 close(fbfd);
 getchar();
 return 0;
}
/*24位色转成16位色*/
unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)
 {
     unsigned short  B = (blue >> 3) & 0x001F;
     unsigned short  G = ((green >> 2) << 5) & 0x07E0;
     unsigned short  R = ((red >> 3) << 11) & 0xF800;
     return (unsigned short) (R | G | B);
  }
