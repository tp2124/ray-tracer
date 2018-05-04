#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

#include <time.h>


#include "RenderingSettings.h"

#include "3DGeometry.h"
#include "Utility.h"

#include <cmath>
#include "Ray.h"
#include "Vector3.h"
#include "Matrix.h"


char *filename=0;
void save_jpg();

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_DISPLAY;

Vector3<> g_ScreenColor[HEIGHT][WIDTH];
Vector3<> g_PostAliasScreenColor[HEIGHT][WIDTH];

unsigned char buffer[HEIGHT][WIDTH][3];

float g_fCamXScreenBound;
float g_fCamYScreenBound;

template<class T>
Vector3<> screenToWorld(T x, T y);

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Geometry* allGeometry[MAX_TRIANGLES + MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

Vector3<> background(0.3f, 0.3f, 0.3f);//Vector3<>(0.7f, 0.7f, 0.7f);

// Attenuation 
const float attenuationA = 0.5f;
const float attenuationB = 0.01f;
const float attenuationC = 0.01f;

void applyAttenuation(Vector3<>& color, float distanceToLight);
//------------------


// Anti-Aliasing

float g_fMaxAliasPixelCoordinateDeviation = 0.05f;//0.05f;

//------------------

template<class T>
void scaleColorValues(T(&color)[3]);
void scaleColorValues(Vector3<>& color);

int num_triangles=0;
int num_spheres=0;
int num_geometry=0;
int num_lights=0;

Vector3<> g_vCameraPosition; //= Vector3<>(-5, -2, 5);

float g_fCameraMoveSpeed = 0.15f;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x, int y, Vector3<> color);

void rayTracePixel(int x, int y);

const float g_fRecursionLightFraction = 0.9f;//0.05f;//0.1f;
const int g_iMaxRayRecursions = 10;
bool rayTrace(const Ray& ray, Vector3<>& color, int recursionLevel = 0, double indexOfRefraction = 1);

const float g_fMinRayHitTime = 0.001f;
const float g_fMaxRayHitTime = 5000;
double testRayCollision(const Ray& ray, Geometry*& closestGeometry, double maxRayTime = g_fMaxRayHitTime, Geometry* geometryToIgnore = NULL);

void antiAlias();

inline float calculateSobelAt(int x, int y);
const float g_fMinSobelEdgeVal = 0.5f;

//MODIFY THIS FUNCTION
void draw_scene()
{
	float aspectRatio = WIDTH/(float)HEIGHT;

	float fovDegree = fov;
	float fovRad = degreeToRadian(fov);
	g_fCamXScreenBound = aspectRatio * tan(fovRad/2);
	g_fCamYScreenBound = tan(fovRad/2);

	Vector3<> rayDir;
	Vector3<> color;

	double fstart, fstop;	
	glPointSize(2.0);  

	for(unsigned int x=0; x<WIDTH; x++)
	{ 
		glBegin(GL_POINTS);
		
		for(unsigned int y=0;y < HEIGHT;y++)
		{
			rayTracePixel(x, y);
			plot_pixel(x, y, Vector3<>(g_ScreenColor[y][x]));
		}

		glEnd();
		glFlush();
	}

#if ANTI_ALIASING
	antiAlias();

	for(unsigned int x=0; x<WIDTH; x++)
	{
		glPointSize(2.0);  
		glBegin(GL_POINTS);

		for(unsigned int y=0;y < HEIGHT;y++)
		{
			plot_pixel(x, y, g_PostAliasScreenColor[y][x]);
		}

		glEnd();
		glFlush();
	}
#endif

	if(mode == MODE_JPEG)
	{
		save_jpg();
	}

	printf("Done!\n"); fflush(stdout);
}

template<class T>
Vector3<> screenToWorld(T x, T y)
{
	return Vector3<>(-g_fCamXScreenBound + x/(float)WIDTH * g_fCamXScreenBound * 2, -g_fCamYScreenBound + y/(float)HEIGHT * g_fCamYScreenBound * 2, -1) + g_vCameraPosition;
}

void applyAttenuation(Vector3<>& color, float distanceToLight)
{
	color/=(attenuationA + attenuationB * distanceToLight + attenuationC * distanceToLight * distanceToLight);
}

template<class T>
void scaleColorValues(T(&color)[3])
{
	for(int i = 0; i < 3; i++) 
	{
		color[i] = clamp(color[i], 0, 1);
		color[i] *= 255;
	}
}

void scaleColorValues(Vector3<>& color)
{
	scaleColorValues(color.values);
}

void rayTracePixel(int x, int y)
{
	Vector3<> rayDir;
	Vector3<> color;

#if DEBUG
	//This statement used to look up specific points when debugging. Breakpoints put in if
	if(x == 193 && y == 120)
	{
		int j;
		j = x;
	}
#endif

	Vector3<> worldScrenPos = screenToWorld(x, y);
	rayDir = worldScrenPos - g_vCameraPosition;
	Ray ray = Ray(rayDir, g_vCameraPosition);

	rayTrace(ray, color);

	g_ScreenColor[y][x] = color;
}

// returns true if hit any geometry
bool rayTrace(const Ray& ray, Vector3<>& color, int recursionLevel, double indexOfRefraction)
{
	recursionLevel++;
	if(recursionLevel > g_iMaxRayRecursions)
	{
		// add to specular?
		return true;
	}
	Geometry* closestGeometry = NULL;
	double hitTime = testRayCollision(ray, closestGeometry);

	if(hitTime > 0 && closestGeometry)
	{
		// -----------------------------
		// AMBIENT ILLUMINATION
		for(int i = 0; i<3; i++)
		{
			color[i] += ambient_light[i];
		}
		// -----------------------------

		Vector3<> hitPos = ray.getPosAtTime(hitTime);

		Vector3<> geometryNormal = closestGeometry->getNormalAtPoint(hitPos);
		MaterialInfo& geometryMaterial = closestGeometry->getMaterialInfoAtPoint(hitPos);

#if TRANSLUCENCY

		if(geometryMaterial.translucent)
		{
			double fraction, nextIndexOfRefraction, normalMultiplier, indexDifference;
			// hack to know if coming out of geometry
			if(indexOfRefraction == geometryMaterial.indexOfRefraction)
			{
				nextIndexOfRefraction =  1; // air
				normalMultiplier = -1;
			}
			else
			{
				nextIndexOfRefraction = geometryMaterial.indexOfRefraction; // inside object
				normalMultiplier = 1;
			}

			Vector3<> geometryRefractionNormal = geometryNormal * normalMultiplier;

			fraction = indexOfRefraction / nextIndexOfRefraction;

			indexDifference = nextIndexOfRefraction - indexOfRefraction;

			// referenced approximation from http://www-cs-students.stanford.edu/~adityagp/acads/final/node5.html
			Vector3<> transmittedDir = ray.getDirection() + geometryRefractionNormal*indexDifference;



			Ray transmitionRay(transmittedDir, hitPos);
			rayTrace(transmitionRay, color, recursionLevel, 1/*nextIndexOfRefraction*/); // just goign to assume that the ray will always be in air after
		} 
		//else

#endif
		{
			for(int i = 0; i < num_lights; i++)
			{
				Vector3<> dirToLight = lights[i].position - hitPos;
				double distanceToLight = dirToLight.getLength();

				dirToLight.normalize();
				Ray rayToLight = Ray(dirToLight, hitPos);
				Geometry* blockingGeometry = NULL;
				double blockingTime = testRayCollision(rayToLight, blockingGeometry, distanceToLight);			

				// No hit, light can get through
				if(blockingTime == -1)
				{
					Vector3<> colorContributionFromLight;

					// -----------------------------
					// Diffuse
					double lDotN = rayToLight.getDirection().dot(geometryNormal);
					if(lDotN > 0)
					{
						float diffuse[3] = {0,0,0};
							
						for(int j = 0; j<3; j++)
						{
							diffuse[j] = geometryMaterial.color_diffuse[j] * lights[i].color[j] * (lDotN) * geometryMaterial.colorTranslucency;
							colorContributionFromLight[j] += diffuse[j];
						}
					}
					// -----------------------------

					// -----------------------------
					// Specular
					Vector3<> dirToCam = Vector3<>() - hitPos;
					dirToCam.normalize();

					Vector3<> reflectionVector = dirToLight.getReflectedVector(geometryNormal, true);

					float vDotR = dirToCam.dot(reflectionVector);
					if(vDotR > 0)
					{
#if RECURSIVE_SPECULAR
						Vector3<> reflectedColor, specular;
						Ray reflectionRay = Ray(reflectionVector, hitPos);

						if(rayTrace(reflectionRay, reflectedColor, recursionLevel))
						{
							if(recursionLevel > 1)
							{
								int f;
							}
							//reflectedColor*= g_fRecursionLightFraction;
						
							for(int j = 0; j<3; j++)
							{
								specular[j] = pow(vDotR, (float)geometryMaterial.shininess) * geometryMaterial.color_specular[j] * reflectedColor[j] * g_fRecursionLightFraction;
								colorContributionFromLight[j] += specular[j];
							}

							//color += reflectedColor;
						}

	#else
						float specular[3] = {0,0,0};

						for(int j = 0; j<3; j++)
						{
							specular[j] = pow(vDotR, (float)geometryMaterial.shininess) * geometryMaterial.color_specular[j] * lights[i].color[j];
							colorContributionFromLight[j] += specular[j];
						}
	#endif
				
					}	

					// -----------------------------	
				
	#if ATTENUATION
					// Apply attenuation
					applyAttenuation(colorContributionFromLight, distanceToLight);
					// -----------------------------
	#endif

					color+=colorContributionFromLight;
				}
				else // in shadow
				{
				
				}
			}
			// Do not really need this null check
			if(closestGeometry)
			{
				return true;
			}
		}
	}
	else
	{
		// so that the object reflects the background color
		color += background;
	}

	return false;
}

// returns time of ray hit(-1 if none) and sets the closest object hit
//float testRayCollision(const Ray& ray, Sphere*& closestSphere, Triangle*& closestTriangle)
double testRayCollision(const Ray& ray, Geometry*& closestGeometry, double maxRayTime, Geometry* geometryToIgnore)
{
	double hitTime = -1;
	double minHitTime = DBL_MAX;

	int geoIndex = -1;
	for(int i = 0; i < num_geometry; i++)
	{
		// No need for synchronization if parallelized, since geometry is never modified
		Geometry* geo = allGeometry[i];

		if(geo != geometryToIgnore)
		{
			hitTime = ray.getTimeToHit(*geo);
			if(hitTime < minHitTime && hitTime > g_fMinRayHitTime)
			{
				minHitTime = hitTime;
				closestGeometry = geo;
				geoIndex = i;
			}
		}
	}

	if(minHitTime > maxRayTime)
	{
		minHitTime = -1;
	}
	return minHitTime;
}

void antiAlias()
{
	printf("\nStarting Anti-Aliasing!");

	for(unsigned int x=0; x<WIDTH; x++)
	{
		for(unsigned int y=0;y < HEIGHT;y++)
		{
			if(x == 0 || x == WIDTH || y == 0 || y == HEIGHT)
			{
				g_PostAliasScreenColor[y][x] = g_ScreenColor[y][x];
			}
			else
			{
				float fSobelOp = calculateSobelAt(x, y);
				if(fSobelOp > g_fMinSobelEdgeVal)
				{		
					Vector3<> rayDir;
					Vector3<> color;

					float fX, fY;
	
					float fraction = 1 / (float)AA_LEVEL;

					for(float i = -(AA_LEVEL/2); i <= (AA_LEVEL/2); i ++)
					{
						if(i != 0)
						{
							for(float j = -(AA_LEVEL/2); j <= (AA_LEVEL/2); j ++)
							{
								if(j != 0)
								{
									fX = x + i* fraction + getRandomInRange(-g_fMaxAliasPixelCoordinateDeviation, g_fMaxAliasPixelCoordinateDeviation);
									fY = y + j* fraction + getRandomInRange(-g_fMaxAliasPixelCoordinateDeviation, g_fMaxAliasPixelCoordinateDeviation);

									Vector3<> worldScrenPos = screenToWorld(fX, fY);
									rayDir = worldScrenPos - g_vCameraPosition;
									Ray ray = Ray(rayDir, g_vCameraPosition);

									rayTrace(ray, color);
								}
							}
						}
					}

					color/=(AA_LEVEL*AA_LEVEL);

					g_PostAliasScreenColor[y][x] = color;
				}
				else
				{
					g_PostAliasScreenColor[y][x] = g_ScreenColor[y][x];
				}
			}
		}
	}
	printf("\nFinished Anti-Aliasing!\n\n");
}

// used http://www.hackification.com/2008/08/31/experiments-in-ray-tracing-part-8-anti-aliasing/ for reference
inline float calculateSobelAt(int x, int y)
{
	return abs(( g_ScreenColor[y-1][x-1].getValueSum() + 2 * g_ScreenColor[y-1][x].getValueSum() + g_ScreenColor[y-1][x+1].getValueSum()) - ( g_ScreenColor[y+1][x-1].getValueSum() + 2 * g_ScreenColor[y+1][x].getValueSum() + g_ScreenColor[y+1][x+1].getValueSum()))
			+ abs((g_ScreenColor[y-1][x+1].getValueSum() + 2 * g_ScreenColor[y][x+1].getValueSum() + g_ScreenColor[y+1][x+1].getValueSum()) - ( g_ScreenColor[y-1][x-1].getValueSum() + 2 * g_ScreenColor[y][x-1].getValueSum() + g_ScreenColor[y+1][x-1].getValueSum()));
}

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
	glColor3f(((double)r)/256.f,((double)g)/256.f,((double)b)/256.f);
	glVertex2i(x,y);
}

void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
	buffer[HEIGHT-y-1][x][0]=r;
	buffer[HEIGHT-y-1][x][1]=g;
	buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x,int y,unsigned char r,unsigned char g, unsigned char b)
{
	plot_pixel_display(x,y,r,g,b);
	if(mode == MODE_JPEG)
	{
		plot_pixel_jpeg(x,y,r,g,b);
	}
}

void plot_pixel(int x, int y, Vector3<> color)
{
	scaleColorValues(color);
	plot_pixel(x, y, color[0], color[1], color[2]);
}

void save_jpg()
{
	Pic *in = NULL;

	in = pic_alloc(WIDTH, HEIGHT, 3, NULL);
	printf("Saving JPEG file: %s\n", filename);

	memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
	if (jpeg_write(filename, in))
	{
		printf("File saved Successfully\n");
	}
	else
	{
		printf("Error in Saving\n");
	}

	pic_free(in);      

}

void parse_check(char *expected,char *found)
{
	if(stricmp(expected,found))
	{
		char error[100];
		printf("Expected '%s ' found '%s '\n",expected,found);
		printf("Parse error, abnormal abortion\n");
		getchar();
		exit(0);
	}

}

void parse_doubles(FILE*file, char *check, double p[3])
{
	char str[100];
	fscanf(file,"%s",str);
	parse_check(check,str);
	fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
	printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r)
{
	char str[100];
	fscanf(file,"%s",str);
	parse_check("rad:",str);
	fscanf(file,"%lf",r);
	printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi)
{
	char s[100];
	fscanf(file,"%s",s);
	parse_check("shi:",s);
	fscanf(file,"%lf",shi);
	printf("shi: %f\n",*shi);
}

void parse_transCol(FILE*file,double *col)
{
	char s[100];
	fscanf(file,"%s",s);
	parse_check("transCol:",s);
	fscanf(file,"%lf",col);
	printf("transCol: %f\n",*col);
}

int loadScene(char *argv)
{
	FILE *file = fopen(argv,"r");
	int number_of_objects;
	char type[50];
	int i;
	Triangle t;
	Sphere s;
	Light l;
	fscanf(file,"%i",&number_of_objects);

	printf("number of objects: %i\n",number_of_objects);
	char str[200];

	parse_doubles(file,"amb:",ambient_light);

	double double3Array[3];

	// Transperancy material state machine (properties carry on geometry to geometry)
	bool translucent = false;
	double colorTranslucency = 1;

	for(i=0;i < number_of_objects;i++)
	{
		fscanf(file,"%s\n",type);
		printf("%s\n",type);
		if(stricmp(type,"triangle")==0)
		{
			printf("found triangle\n");
			int j;

			for(j=0;j < 3;j++)
			{
				parse_doubles(file,"pos:", double3Array);
				t.v[j].position = Vector3<double>(double3Array);

				parse_doubles(file,"nor:",double3Array);
				t.v[j].normal = Vector3<double>(double3Array);

				parse_doubles(file,"dif:",t.v[j].material.color_diffuse);
				parse_doubles(file,"spe:",t.v[j].material.color_specular);
				parse_shi(file,&t.v[j].material.shininess);

				t.v[j].material.translucent = translucent;
				//Giving it some rate to bend light
				t.v[j].material.indexOfRefraction = 1.49;
				if(translucent)
				{
					t.v[j].material.colorTranslucency = colorTranslucency;
				}
				else
					t.v[j].material.colorTranslucency = 1;
			}

			if(num_triangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				getchar();
				exit(0);
			}
			t.calculatePlaneNormal();
			triangles[num_triangles++] = t;
			allGeometry[num_geometry++] = &triangles[num_triangles-1];
		}
		else if(stricmp(type,"sphere")==0)
		{
			printf("found sphere\n");

			parse_doubles(file,"pos:",s.position);
			parse_rad(file,&s.radius);
			parse_doubles(file,"dif:",s.material.color_diffuse);
			parse_doubles(file,"spe:",s.material.color_specular);
			parse_shi(file,&s.material.shininess);
			
			s.material.translucent = false;

			s.material.translucent = translucent;
			s.material.indexOfRefraction = 1.49;
			if(translucent)
			{
				s.material.colorTranslucency = colorTranslucency;
			}
			else
				s.material.colorTranslucency = 1;

			if(num_spheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				getchar();
				exit(0);
			}
			spheres[num_spheres++] = s;
			allGeometry[num_geometry++] = &spheres[num_spheres-1];
		}
		else if(stricmp(type,"light")==0)
		{
			printf("found light\n");
			parse_doubles(file,"pos:",double3Array);
			l.position = Vector3<>(double3Array);

			parse_doubles(file,"col:",l.color);

			if(num_lights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				getchar();
				exit(0);
			}
			lights[num_lights++] = l;
		}
		else if (stricmp(type,"transperancy")==0)
		{
			printf("found transperancy setting\n");
			
			char str[100];
			fscanf(file,"%s",str);
			if(stricmp(str,"ON")==0)
			{
				translucent = true;
				parse_transCol(file, &colorTranslucency);
				colorTranslucency = clamp(colorTranslucency, 0, 1);
			}
			else if(stricmp(str,"OFF")==0)
			{
				translucent = false;
			}
			else
			{
				printf("Error when trying to parse translucency settings");
				exit(0);
			}

			i--; // this was not an object
		}
		else
		{
			printf("unknown type in scene description:\n%s\n",type);
			getchar();
			exit(0);
		}
	}
	return 0;
}

void display()
{

}

void keyboardDown(unsigned char key, int x, int y)
{
	Vector3<> moveDir;
	switch(key)
	{
		case ' ':
			moveDir = Vector3<>(0, 1, 0);
			break;
		case 'x':
			moveDir = Vector3<>(0, -1, 0);
			break;
		case 'a':
			moveDir = Vector3<>(-1, 0, 0);
			break;
		case 'd':
			moveDir = Vector3<>(1, 0, 0);
			break;
		case 'w':
			moveDir = Vector3<>(0, 0, -1);
			break;
		case 's':
			moveDir = Vector3<>(0, 0, 1);
			break;
	}

	g_vCameraPosition +=moveDir * g_fCameraMoveSpeed;
	draw_scene();
}

void init()
{
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,WIDTH,0,HEIGHT,1,-1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
	//hack to make it only draw once
	static int once=0;
	if(!once)
	{
		draw_scene();
	}
	once=1;
}

int main (int argc, char ** argv)
{
	srand((unsigned)time(0));

	if (argc<2 || argc > 3)
	{  
		printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
		exit(0);
	}
	if(argc == 3)
	{
		mode = MODE_JPEG;
		filename = argv[2];
	}
	else if(argc == 2)
	{
		mode = MODE_DISPLAY;
	}

	glutInit(&argc,argv);
	loadScene(argv[1]);

	//scaleColorValues(background);

	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(WIDTH,HEIGHT);
	int window = glutCreateWindow("Ray Tracer");
	glutKeyboardFunc(keyboardDown);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	init();
	glutMainLoop();
}
