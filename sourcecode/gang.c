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
/*�Զ�д��ʽ����ʾ�豸�ڵ��ļ�/dev/fb0*/
 fbfd=open("/dev/fb0",O_RDWR);
 if(fbfd<=0){
 printf("error:can not open framebuffer device.\n");
 return -1;
 }
 printf("the framebuffer device was opened successfully.\n");
/*�õ�framebuffer�豸�Ĺ̶���Ļ��Ϣ
*struct fb_fix_screeninfo �ṹ������Ҫ�ĳ�Ա��
* unsigned long seme_start;��������������ʼ��ַ�������ַ��
*__u32  smem_len;��������ȡ�ĳ��ȣ�
*����u32  type;����fb���ͣ�����TFT��STN����
*__u32  visual;������ʾ��ɫ�����ɫ��α��ɫ�����ǵ�ɫ
*�ýṹ�����������豸�޹أ����ɱ������Ϣ���û�����ʹ��FBIOGET��FSCREENINFO�����������Щ��Ϣ
*�������£�
*/
//������û���õ��̶������������£�
// if(ioctl(fbfd,FBIOGET_FSCREENINFO,&finfo)){
//  printf("error reading fixed information.\n");
//  return -2;
// }
/*�õ��ɱ���Ļ��Ϣ*/
/*_
*fb_var_screeninfo �ṹ��Ա���£�
*__u32 xres;     //�ɼ��ֱ���xres*yres ��1024*768��800*600�ȼ�Ϊ��˵�����أ�Ҳ�ɿ�����ʾ����
*__u32 yres;      x �� y �ᡣ
*__u32 xres_virtual;//����ֱ���
*__u32 yres_virtual;
*__u32 xoffset;//�����⵽�ɼ��ֱ��ʵ�ƫ��
*__u32 yoffset;
*�Լ���Ļ���ܵ� margin���߾ࣩ������ʱ�ӣ�ͬ��ʱ��ȱ�����Ϣ��
*�ýṹ�����豸�޹صģ��ɸ��ĵ�������Ϣ��Ӧ�ó������ʹ��FBIOGET��VSCREENINFO��������Щ��Ϣ
*�������£�
*ʹ��FBIOPUT_VSCREENINFO����д����Щ��Ϣ��������û�Թ���
*/
 if(ioctl(fbfd,FBIOGET_VSCREENINFO,&vinfo)){
  printf("error reading variable information.\n");
  return -3;
 }
/*��ӳ����Դ�Ĵ�С*/ 
//bits_per_pixle  ����λɫ��32λ��24λ��16λ��ÿ������λɫ��
 screensize = vinfo.xres*vinfo.yres*vinfo.bits_per_pixel/8;
/*���Դ�ӳ���û��ռ�*/
 fbp=(char*)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fbfd,0);
 if((int)fbp==-1){
 printf("error:failed to map framebuffer device to memory.\n");
 return -4;
 }
 printf("the framebuffer device was mapped to memory successfully.\n");
 
 
 FILE *infp;
 unsigned char *buffer;
/*   ��Ҫ��ʾ��ͼƬ*/
 if ((infp=fopen(argv[1],"r"))==NULL)
 {
  printf("Error:open %s failed\n",argv[1]);
  exit(0);
 } 
/*��ȡͼƬ����*/
/*
*��ѹ��������ʹ�õ�JPEG������һ��jpeg_decompress_struct�Ľṹ�塣ͬʱ����Ҫ����һ�����ڴ�����Ľṹ�����IJG�б�׼�Ĵ���ṹ����jpeg_error_mgr��
*struct jpeg_decompress_struct cinfo; 
*struct jpeg_error_mgr jerr;
*Ȼ���ǽ�������ṹ�������JPEG�����ϡ�
*cinfo.err = jpeg_std_error(&jerr);
*�����׼�Ĵ�����ṹ��ʹ�����ڳ��ִ���ʱ����exit()�˳����������ϣ��ʹ�ñ�׼�Ĵ�����ʽ�������ͨ���Զ����˳������ķ����Զ��������ṹ
*/
 cinfo.err=jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);//��ʼ��cinfo�ṹ
/*���ñ�׼C�е��ļ�ָ�봫��Ҫ�򿪵�jpg�ļ���
* FILE * infile;
*if ((infile = fopen("sample.jpg", "rb")) == NULL) { return 0; }
* jpeg_stdio_src(&cinfo, infile);  
*/
    jpeg_stdio_src(&cinfo, infp);
/*��ͼ���ȱʡ��Ϣ��䵽cinfo�ṹ���Ա����ʹ�á�
  (void) jpeg_read_header(&cinfo, TRUE);  
*��ʱ�������Ŀ�����Ϣ����ͼ��Ŀ�cinfo.image_width����cinfo.image_height��ɫ�ʿռ�cinfo.jpeg_color_space����ɫͨ����cinfo.num_components�ȡ�
*/
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);//��ʼ��ѹ������ɽ�ѹ��������IJG�ͻὫ��ѹ���ͼ����Ϣ�����cinfo�ṹ�С����磬���ͼ����cinfo.output_width�����ͼ��߶�cinfo.output_height��ÿ�������е���ɫͨ����cinfo.output_components������Ҷ�Ϊ1��ȫ��ɫΪ3���ȡ�
/*output_widthͼƬʵ�ʿ�output_height ͼƬʵ�ʸ߶�*/
 if ((cinfo.output_width > vinfo.xres) || (cinfo.output_height > vinfo.yres))
  {
   printf("too large JPEG file,cannot display\n");
   return (-1);
     }
/*output_componentsͼƬ������*ÿ�е�output_width����ͼƬ�Ĵ�С*/
    buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_components);
 y=0;
/*��ͼ*/
  while (cinfo.output_scanline < cinfo.output_height) 
 {
         jpeg_read_scanlines(&cinfo, &buffer, 1);//ɨ����
         if (vinfo.bits_per_pixel== 16) //�����ʾ�豸ʱ16λɫ������Ҫ�Ѳɼ���24λɫת����16λɫ��
   {
          unsigned short  color;//ÿ���ص�RGB
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
/*24λɫת��16λɫ*/
unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)
 {
     unsigned short  B = (blue >> 3) & 0x001F;
     unsigned short  G = ((green >> 2) << 5) & 0x07E0;
     unsigned short  R = ((red >> 3) << 11) & 0xF800;
     return (unsigned short) (R | G | B);
  }
