#import <Cocoa/Cocoa.h>

typedef enum
{
	redIndex,
	greenIndex,
	blueIndex,
	alphaIndex
} ClearColors;

@interface MyOpenGLView : NSOpenGLView
{
    NSTimer *timer;
    uint8_t *buffer;
}
@end
