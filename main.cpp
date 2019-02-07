/***************************************************************
| Joseph B. Mattie
| CS481
| OpenFPS
\**************************************************************/
#include <iostream>
#include <fstream>
	using namespace std;
#include <string.h>
	using std::string;
#include <math.h>
#include <float.h>
#include <GL/glut.h>
#include <al/al.h>
#include <al/alut.h>
#define DEBUG 0
#define PI 3.1415926535897932
//#define PI 3.14159
#define BEHIND		0
#define INTERSECTS	1
#define FRONT		2

#define DRAW_COLLISION_POINTS 0

#define NUM_SOUND_BUFFERS 9
#define NUM_SOUND_SOURCES 4
#define NUM_SOUND_ENVIRONMENTS 1

void init(void);

ALuint SOUNDbuffer[NUM_SOUND_BUFFERS];
ALuint SOUNDsource[NUM_SOUND_SOURCES];
ALuint SOUNDenvironment[NUM_SOUND_ENVIRONMENTS];

ALfloat SOUNDlistenerP[]={0.0,0.0,0.0};
ALfloat SOUNDlistenerV[]={0.0,0.0,0.0};
ALfloat	SOUNDlistenerO[]={0.0,0.0,1.0, 0.0,1.0,0.0};

ALsizei SOUNDsize;
ALsizei SOUNDfreq;

ALenum SOUNDformat;
ALvoid *SOUNDdata;

bool Audio = true;

int windowID;
bool fullscreen = false;
int windowWidth = 640;
int windowHeight = 480;
char screenSettings[5][80];
int screenResIndex = 0;

const double nClip = 0.1;
const double fClip = 500.0;

GLdouble view_matrix[16];
GLdouble mouseMult = 500;
double gravity = 0.3;

int moveUD = 0;
int moveLR = 0;

int actualX,actualY;
int mouseX, mouseY;

struct pointStruct 
{
	GLdouble X,Y,Z;
};

struct cylStruct
{
	GLdouble bottom, top, radius;
};
struct colorStruct 
{
	GLdouble R,G,B,A;
};

struct texCoordStruct
{
	GLdouble X,Y;
};

struct faceStruct
{
	int numPoints;
	int *vertIndex;
	int *normIndex;
	int *texIndex;
	bool collision;
	bool visible;
};

struct objectStruct 
{
	char *name;
	int numFaces;
	int firstFace;
	int material;
};

struct frameStruct
{
	int totalVertices;
	int totalTexCoords;
	int totalNormals;
	int totalFaces;

	faceStruct *faces;
	pointStruct *vertices;
	pointStruct *normals;
	texCoordStruct *texCoords;
};

struct animationStruct
{
	char *name;
	int numFrames;
	char *aninamebase;

	frameStruct *frames;
	int currentframe;
};

struct modelStruct
{
	char *name;
	char *filenamebase;
	GLdouble radius;
	int numAnimations;
	int material;
	animationStruct *animations;
	int currentanimation;
};

struct entityStruct
{
	char *type;
	colorStruct color;
	colorStruct ambientcolor;
	pointStruct point;
};

struct materialStruct 
{
	char name[255];
	int texture;
	colorStruct ambient;
	colorStruct diffuse;
	colorStruct specular;
	GLdouble shine;
};

struct textureStruct 
{
	char *name;
	char *data;
	long size;
};

struct menuItemStruct
{
	char *text;
	char *command;
};

struct menuStruct 
{
	char *name;
	char *text;
	menuItemStruct *menuItems;
	int numMenuItems;
	bool fixedfont;
};

struct enemyStruct
{
	pointStruct position;
	int modelIndex;
	int rotation;
	bool gravity;
	bool clipping;
	GLdouble attackDistance;
	GLdouble speed;
	int currentanimation;
	int health;
	bool dying;
	int attackDamage;
	int state;
	
	// 0 = idle
	// 1 = sighted
	// 2 = hunting
	// 3 = attacking
};

struct keyStruct
{
	char walkUp;
	char walkDown;
	char strafeL;
	char strafeR;
	char jump;
	char cameraMode;
	char console;
	char menu;
};


keyStruct keys;
bool getNextKeypress = false;
char *keytoset;


int totalVertices = 0;
int totalTexCoords = 0;
int totalNormals = 0;
int totalFaces = 0;
int totalObjects = 0;
int totalEntities = 0;
int totalMaterials = 0;
int totalTextures = 0;
int totalModels = 0;
int totalMenus = 0;
int totalEnemies = 0;

GLuint texture[128];

menuStruct *menus;
pointStruct *vertices;
pointStruct *normals;
texCoordStruct *texCoords;

faceStruct *faces;
objectStruct *objects;

modelStruct *models;

entityStruct *entities;
enemyStruct *enemies;

textureStruct *textures;

materialStruct *materials;
materialStruct defaultMaterial;

pointStruct playerPosition;
pointStruct playerView;
pointStruct playerUp;

pointStruct oldPlayerPosition;


GLdouble playerRotation = 0.0;

float playerSpeed = 0.1;
float currentSpeed = 0.0;
GLdouble playerRadius = 0.5;
bool playerJump = false;
double playerJumpThrust = 0.45;
double playerJumping = 0.0;
int playerModelIndex = 0;
GLdouble playerGroundOffset;
int playerHealth;
int playerRecoverTime = 0;
bool playerDead = 0;
int gunDamage = 10;

int viewMode = 1;

bool showMenu = false;
int showMenuIndex = 0;
int menuHeadingSize = 12;
int menuHeadingOffset = 10;
int menuItemSize = 12;
int menuItemOffset = 10;
int menuBGwidth=512;
int menuBGheight=384;
int menuHover = 0;
bool menuClick = false;

bool console = false;
int consoleBufferLength = 100;
char consolebuffer[100][80];
int consoleBufferCurrentLength=0;

char consoleCommand[80];
int consoleCursorPosition = 0;

GLdouble mouseXDelta;

bool lighting = true;
bool physics = true;
bool clipping = true;


void enemyDie(int enemy)
{

	enemies[enemy].currentanimation=2;
	enemies[enemy].dying = true;
	alSourceStop(SOUNDsource[NUM_SOUND_SOURCES+enemy]);
}

void Die()
{
	playerDead = true;
	playerHealth=0;
	if(Audio)
	{
		alSourceStop(SOUNDsource[2]);
		alSourcei(SOUNDsource[2],AL_BUFFER,SOUNDbuffer[0]);
		alSourcePlay(SOUNDsource[2]);
	}
	playerUp.X=1.0;
	playerUp.Y=0.0;
	playerUp.Z=0.0;
}

void Live()
{
	playerDead = false;
	playerHealth = 100;
	playerUp.X=0.0;
	playerUp.Y=1.0;
	playerUp.Z=0.0;
}

void showHealthMeter()
{
	char health[10];
	int i;

	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, actualX, 0, actualY);
	glScalef(1, -1, 1);
	glTranslatef(0, -actualY, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	glColor3d(1.0,playerHealth/100.0,playerHealth/100.0);
	glRasterPos2f(actualX-100,actualY-50);

	strcpy(health,"Health: ");

	for(i=0; i<strlen(health); i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, health[i]);
	
	_itoa(playerHealth, health, 10);

	for(i=0; i<strlen(health); i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, health[i]);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
}

void showCrossHairs()
{
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, actualX, 0, actualY);
	glScalef(1, -1, 1);
	glTranslatef(0, -actualY, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	glColor3d(1,1,1);

	glBegin(GL_LINES);
		glVertex2d(actualX/2-10,actualY/2);
		glVertex2d(actualX/2+10,actualY/2);

		glVertex2d(actualX/2,actualY/2-10);
		glVertex2d(actualX/2,actualY/2+10);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
}



void toConsole(char *string)
{
	if(consoleBufferCurrentLength>=consoleBufferLength)
	{
		for(int i=0;i<consoleBufferLength;++i)
		{
			strcpy(consolebuffer[i],consolebuffer[i+1]);
		}
		strcpy(consolebuffer[consoleBufferLength],string);
	}
	else
	{
		strcpy(consolebuffer[consoleBufferCurrentLength],string);
		consoleBufferCurrentLength++;
	}
}

void toConsoleCat(char *string)
{
	strcat(consolebuffer[consoleBufferCurrentLength-1],string);
}

void drawConsole()
{
	int cursorPos;
	int i;

	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, actualX, 0, actualY);
	glScalef(1, -1, 1);
	glTranslatef(0, -actualY, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3d(0.1,0.1,0.1);
	glBegin(GL_QUADS);
		glVertex2d(0,0);
		glVertex2d(actualX,0);
		glVertex2d(actualX,actualY/2+20);
		glVertex2d(0,actualY/2+20);
	glEnd();
	glColor3d(0.8,0.8,0.8);
	glBegin(GL_QUADS);
		glVertex2d(0,actualY/2+2);
		glVertex2d(actualX,actualY/2+2);
		glVertex2d(actualX,actualY/2+18);
		glVertex2d(0,actualY/2+18);
	glEnd();
	glColor3d(1,1,1);

	for(i=0; i<consoleBufferCurrentLength; i++)
	{
		glRasterPos2f(3,actualY/2-(11*(consoleBufferCurrentLength-i)));
		for(int j=0; j<80; j++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, consolebuffer[i][j]);
		}
		
	}
	glColor3d(0,0,0);
	glRasterPos2f(3,actualY/2+16);


	cursorPos=3;
	for(i=0; i<80; i++)
	{
		cursorPos+=glutBitmapWidth(GLUT_BITMAP_HELVETICA_10, consoleCommand[i]);
		if(consoleCursorPosition==i)
		{
			glColor3d(0.1,0.9,0.1);
			glBegin(GL_QUADS);
				glVertex2d(cursorPos-glutBitmapWidth(GLUT_BITMAP_HELVETICA_10, consoleCommand[i]),actualY/2+6);
				glVertex2d(cursorPos,actualY/2+6);
				glVertex2d(cursorPos,actualY/2+16);
				glVertex2d(cursorPos-glutBitmapWidth(GLUT_BITMAP_HELVETICA_10, consoleCommand[i]),actualY/2+16);
			glEnd();
			glColor3d(1,1,1);
		}
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, consoleCommand[i]);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
}

GLdouble absolute(GLdouble value)
{
	if(value>0.0)
		return value;
	else
	return value*-1.0;
}

void loadTexture(int textureIndex)
{
	ifstream textureFile(textures[textureIndex].name);
	if (!textureFile.is_open())
		cout << "Could not open File" << endl;
	else
	{
		textureFile.seekg (0, ios::end);
		textures[textureIndex].size = textureFile.tellg();
		textures[textureIndex].data = new char[textures[textureIndex].size];
		textureFile.seekg (0, ios::beg);
		textureFile.read (textures[textureIndex].data,textures[textureIndex].size);
		textures[textureIndex].size = (long)sqrt((double)textures[textureIndex].size/3.0);
		textureFile.close();
	}
}


void dumpLevelInfo()
{
	int i;

	cout << "Vertices:  " << totalVertices << endl;
	cout << "Normals:   " << totalNormals << endl;
	cout << "TexCoords: " << totalTexCoords << endl;
	cout << "Faces:     " << totalFaces << endl;
	cout << "Objects:   " << totalObjects << endl;
	cout << "Entities:  " << totalEntities << endl;
	cout << "Materials: " << totalMaterials << endl;

	for (i=0; i<totalObjects; ++i)
	{
		cout << "Object #" << i;
		cout << " Face Range: " << objects[i].firstFace << "-" << objects[i].firstFace+objects[i].numFaces << endl;
		
	}

	for (i=0; i<totalEntities; ++i)
	{
		cout << "Entitiy #" << i << " ";
		cout << entities[i].type << ": {" << entities[i].point.X << ":" << entities[i].point.Y << ":" << entities[i].point.Z << "}" << endl;
	}

	for (i=0; i<totalTextures; ++i)
	{
		cout << "Texture #" << i << " ";
		cout << textures[i].name << " is " << textures[i].size << endl;
	}
}

void loadLevel(char *levelname)
{
	char buffer[255];
	char indexbuffer[255];
	char *stop;
	int size = 0;
	int position = 0;
	int jumpto = 0;

	int toCutTo = 0;
	int tempInt = 0;

	int currentVertex = 0;
	int currentNormal = 0;
	int currentTexCoord = 0;

	int currentFace = 0;
	int currentObject = 0;

	bool validLine = false;
	bool startNewObject = false;

	int i;
	
	ifstream level(levelname);

	if (!level.is_open())
		cout << "Could not open File";
	else
	{
		objects[currentObject].name = new char[5];
		strcpy(objects[currentObject].name, "NULL");

		level.seekg (0, ios::end);
		size = level.tellg();
		level.seekg (0, ios::beg);

		do
		{
			validLine = false;
			while(validLine == false)
			{
				jumpto = level.tellg();
				level.getline(buffer, 255, '\n');	
				for(unsigned int i=0; i<strlen(buffer); ++i)
					if (buffer[i]==' ')
						validLine = true;
			}
			level.seekg(jumpto);
			strcpy(indexbuffer,buffer);
			level.getline(buffer, 255, ' ');

			switch(indexbuffer[0])
			{
			case '#':
				level.getline(buffer, 255, '\n');
				break;
			case 'g':
				level.getline(buffer, 255, '\n');
				objects[currentObject].name = new char[strlen(buffer)+1];
				strcpy(objects[currentObject].name,buffer);
				currentObject++;
				startNewObject=false;
				break;
			case 'v':
				switch(indexbuffer[1])
				{
				case 't':
					level.getline(buffer, 255, ' ');
					texCoords[currentTexCoord].X = strtod(buffer, &stop);
					level.getline(buffer, 255, '\n');
					texCoords[currentTexCoord].Y = strtod(buffer, &stop);
					currentTexCoord++;
					break;
				case 'n':
					level.getline(buffer, 255, ' ');
					normals[currentNormal].X = strtod(buffer,&stop);
					level.getline(buffer, 255, ' ');
					normals[currentNormal].Y = strtod(buffer,&stop);
					level.getline(buffer, 255, '\n');
					normals[currentNormal].Z = strtod(buffer,&stop);
					currentNormal++;
					break;
				case ' ':
				default:
					if(startNewObject==true)
					{
						startNewObject=false;
						currentObject++;
						objects[currentObject].name = new char[5];
						strcpy(objects[currentObject].name, "NULL");
					}
					level.getline(buffer, 255, ' ');
					//cout << "Was:" << buffer << "-";
					vertices[currentVertex].X = strtod(buffer,&stop);
					level.getline(buffer, 255, ' ');
					//cout << buffer << "-";
					vertices[currentVertex].Y = strtod(buffer,&stop);
					level.getline(buffer, 255, '\n');
					//cout << buffer << endl;
					vertices[currentVertex].Z = strtod(buffer,&stop);
					//cout << "Is: " << vertices[currentVertex].X << "-" << vertices[currentVertex].Y << "-" << vertices[currentVertex].Z << endl;

					currentVertex++;
					break;
				}
				break;
			case 'f':				
				if(startNewObject==false)
				{
					objects[currentObject-1].firstFace=currentFace;
					objects[currentObject-1].numFaces = 0;
					startNewObject=true;
				}
			
				jumpto = level.tellg();
				level.getline(buffer, 255, '\n');
				
				faces[currentFace].numPoints=0;
				for(i=0; i<strlen(buffer); ++i)
					if (buffer[i]==' ')
						faces[currentFace].numPoints++;

				level.seekg(jumpto);

				faces[currentFace].numPoints++;

				faces[currentFace].vertIndex = new int[faces[currentFace].numPoints];
				faces[currentFace].normIndex = new int[faces[currentFace].numPoints];
				faces[currentFace].texIndex = new int[faces[currentFace].numPoints];
				
				for(i=0; i<faces[currentFace].numPoints-1; ++i)
				{
					level.getline(buffer, 255, '/');
					faces[currentFace].vertIndex[i] = strtol(buffer,&stop,10)-1;
					level.getline(buffer, 255, '/');
					faces[currentFace].normIndex[i] = strtol(buffer,&stop,10)-1;
					level.getline(buffer, 255, ' ');
					faces[currentFace].texIndex[i] = strtol(buffer,&stop,10)-1;
				}

				level.getline(buffer, 255, '/');
				faces[currentFace].vertIndex[faces[currentFace].numPoints-1] = strtol(buffer,&stop,10)-1;
				level.getline(buffer, 255, '/');
				faces[currentFace].normIndex[faces[currentFace].numPoints-1] = strtol(buffer,&stop,10)-1;
				level.getline(buffer, 255, '\n');
				faces[currentFace].texIndex[faces[currentFace].numPoints-1] = strtol(buffer,&stop,10)-1;
				objects[currentObject-1].numFaces++;
				currentFace++;
				break;
			default:
				break;
			}
			position = level.tellg();
		}
		while(position < size);
	}
}


void getFrameInfo(frameStruct &frame, char *modelname)
{
	char buffer[255];
	int size = 0;
	int position = 0;

	frame.totalFaces = 0;
	frame.totalNormals = 0;
	frame.totalTexCoords = 0;
	frame.totalVertices = 0;
	
	ifstream model(modelname);

	if (!model.is_open())
		cout << "Could not open File";
	else
	{
		model.seekg (0, ios::end);
		size = model.tellg();
		model.seekg (0, ios::beg);
		do
		{
			model.getline(buffer, 255, '\n');
			switch(buffer[0])
			{
			case 'v':
				switch(buffer[1])
				{
				case 't':
					frame.totalTexCoords++;					
					break;
				case 'n':
					frame.totalNormals++;
					break;
				case ' ':
					frame.totalVertices++;
					break;
				}
				break;
			case 'f':
				frame.totalFaces++;
				break;
			default:
				break;
			}
		position = model.tellg();
		} while(position < size);
	}
}

void loadFrame(frameStruct &frame, char *modelname)
{
	char buffer[255];
	char indexbuffer[255];
	char *stop;
	int size = 0;
	int position = 0;
	int jumpto = 0;

	int toCutTo = 0;
	int tempInt = 0;

	int currentVertex = 0;
	int currentNormal = 0;
	int currentTexCoord = 0;

	int currentFace = 0;
	int newSize =0;

	bool validLine = false;

	int i;

	getFrameInfo(frame, modelname);

	frame.texCoords = new texCoordStruct[frame.totalTexCoords];
	frame.vertices = new pointStruct[frame.totalVertices];
	frame.normals = new pointStruct[frame.totalNormals];
	frame.faces = new faceStruct[frame.totalFaces];

	ifstream model(modelname);
	
	if (!model.is_open())
		cout << "Could not open File";
	else
	{		
		model.seekg (0, ios::end);
		size = model.tellg();
		model.seekg (0, ios::beg);

		do
		{
			validLine = false;
			while(validLine == false &&  position<size)
			{
				position = model.tellg();
				jumpto = model.tellg();
				model.getline(buffer, 255, '\n');	
				for(unsigned int i=0; i<strlen(buffer); ++i)
					if (buffer[i]==' ')
						validLine = true;
			}

			model.seekg(jumpto);
			strcpy(indexbuffer,buffer);
			model.getline(buffer, 255, ' ');
			
			switch(indexbuffer[0])
			{
			case '#':
				model.getline(buffer, 255, '\n');
				break;
			case 'g':
				model.getline(buffer, 255, '\n');				
				break;
			case 'v':
				switch(indexbuffer[1])
				{
				case 't':
					model.getline(buffer, 255, ' ');
					
					frame.texCoords[currentTexCoord].X = strtod(buffer, &stop);
					model.getline(buffer, 255, '\n');
					frame.texCoords[currentTexCoord].Y = strtod(buffer, &stop);
					currentTexCoord++;
					break;
				case 'n':
					model.getline(buffer, 255, ' ');
					frame.normals[currentNormal].X = strtod(buffer,&stop);
					model.getline(buffer, 255, ' ');
					frame.normals[currentNormal].Y = strtod(buffer,&stop);
					model.getline(buffer, 255, '\n');
					frame.normals[currentNormal].Z = strtod(buffer,&stop);
					currentNormal++;
					break;
				case ' ':
				default:
				/*	if(modelID==playerModelIndex)
					{
						model.getline(buffer, 255, ' ');

						if(playerRadius<absolute(strtod(buffer,&stop)))
							playerRadius=absolute(strtod(buffer,&stop));
						
						models[modelID].vertices[currentVertex].X = strtod(buffer,&stop);
						model.getline(buffer, 255, ' ');

						if(playerRadius<absolute(strtod(buffer,&stop)))
							playerRadius=absolute(strtod(buffer,&stop));

						if(playerGroundOffset>strtod(buffer,&stop))
							playerGroundOffset=strtod(buffer,&stop);

						models[modelID].vertices[currentVertex].Y = strtod(buffer,&stop);
						model.getline(buffer, 255, '\n');

						if(playerRadius<absolute(strtod(buffer,&stop)))
							playerRadius=absolute(strtod(buffer,&stop));
						
						models[modelID].vertices[currentVertex].Z = strtod(buffer,&stop);										
						currentVertex++;
					}
					else */
					model.getline(buffer, 255, ' ');
					frame.vertices[currentVertex].X = strtod(buffer,&stop);

					model.getline(buffer, 255, ' ');					
					frame.vertices[currentVertex].Y = strtod(buffer,&stop);
					model.getline(buffer, 255, '\n');			
					frame.vertices[currentVertex].Z = strtod(buffer,&stop);

					currentVertex++;
					
					break;
				}
				break;
			case 'f':				
				jumpto = model.tellg();
				model.getline(buffer, 255, '\n');				
				frame.faces[currentFace].numPoints=0;
				
				for(i=0; i<strlen(buffer); ++i)
				{
					if (buffer[i]==' ')
						frame.faces[currentFace].numPoints++;
				}
				frame.faces[currentFace].numPoints++;
				model.seekg(jumpto);

				newSize = frame.faces[currentFace].numPoints;				
				frame.faces[currentFace].vertIndex = new int[newSize];
				frame.faces[currentFace].normIndex = new int[newSize];
				frame.faces[currentFace].texIndex = new int[newSize];
				
				for(i=0; i<frame.faces[currentFace].numPoints-1; ++i)
				{
					model.getline(buffer, 255, '/');
					frame.faces[currentFace].vertIndex[i] = strtol(buffer,&stop,10)-1;
					model.getline(buffer, 255, '/');
					frame.faces[currentFace].normIndex[i] = strtol(buffer,&stop,10)-1;
					model.getline(buffer, 255, ' ');
					frame.faces[currentFace].texIndex[i] = strtol(buffer,&stop,10)-1;
				}

				model.getline(buffer, 255, '/');
				frame.faces[currentFace].vertIndex[frame.faces[currentFace].numPoints-1] = strtol(buffer,&stop,10)-1;
				model.getline(buffer, 255, '/');
				frame.faces[currentFace].normIndex[frame.faces[currentFace].numPoints-1] = strtol(buffer,&stop,10)-1;
				model.getline(buffer, 255, '\n');
				frame.faces[currentFace].texIndex[frame.faces[currentFace].numPoints-1] = strtol(buffer,&stop,10)-1;
				
				currentFace++;
				break;
			default:
				break;
			}
		}
		while(position < size);
	}
}

void loadModel(char *modelname)
{
	char buffer[255];
	char *stop;
	char *loadername;
	
	int currentmodel;
	int currentanimation;

	ifstream model(modelname);

	totalModels=0;
	int length;

	if (!model.is_open())
		cout << "Could not open model file";
	else
	{
		do
		{
			model.getline(buffer, 255, '"');
			model.getline(buffer, 255, '"');
			if(strcmp(buffer,"name")==0)
				totalModels++;
		} while (strcmp(buffer,"eof")!=0);
			
		models = new modelStruct[totalModels];
		currentmodel = 0;
		model.seekg(ios::beg);

		do
		{
			model.getline(buffer, 255, '"');
			model.getline(buffer, 255, '"');

			if(strcmp(buffer,"name")==0)
			{
				model.getline(buffer, 255, '"');
				model.getline(buffer, 255, '"');
				models[currentmodel].name = new char[strlen(buffer)+1];
				strcpy(models[currentmodel].name,buffer);
				if(strcmp(buffer,"player")==0)
					playerModelIndex=currentmodel;
				currentmodel++;
			}
			if(strcmp(buffer,"numAnimations")==0)
			{
				model.getline(buffer, 255, '"');
				model.getline(buffer, 255, '"');
				models[currentmodel-1].numAnimations=strtol(buffer, &stop, 10);
			}
		} while (strcmp(buffer,"eof")!=0);

		for (int i=0; i<totalModels; ++i)
			models[i].animations = new animationStruct[models[i].numAnimations];

		currentmodel=0;
		currentanimation=0;
		model.seekg(ios::beg);

		do
		{
			model.getline(buffer, 255, '"');
			model.getline(buffer, 255, '"');

			if(strcmp(buffer,"name")==0)
			{
				currentanimation=0;
				currentmodel++;
			}
			if(strcmp(buffer,"filenameBase")==0)
			{
				model.getline(buffer, 255, '"');
				model.getline(buffer, 255, '"');
				models[currentmodel-1].filenamebase = new char[strlen(buffer)+1];
				strcpy(models[currentmodel-1].filenamebase,buffer);
			}
			if(strcmp(buffer,"radius")==0)
			{
				model.getline(buffer, 255, '"');
				model.getline(buffer, 255, '"');				
				models[currentmodel-1].radius=strtod(buffer, &stop);
				if(playerModelIndex==currentmodel-1)
					playerRadius = models[currentmodel-1].radius;
			}
			
			if(strcmp(buffer,"animationName")==0)
			{
				currentanimation++;
				model.getline(buffer, 255, '"');
				model.getline(buffer, 255, '"');
				models[currentmodel-1].animations[currentanimation-1].name = new char[strlen(buffer)+1];
				strcpy(models[currentmodel-1].animations[currentanimation-1].name,buffer);
			}

			if(strcmp(buffer,"animationFrames")==0)
			{
				model.getline(buffer, 255, '"');
				model.getline(buffer, 255, '"');
				models[currentmodel-1].animations[currentanimation-1].numFrames = strtol(buffer, &stop, 10);
				models[currentmodel-1].animations[currentanimation-1].frames = new frameStruct[models[currentmodel-1].animations[currentanimation-1].numFrames];
			}

			if(strcmp(buffer, "animationFileNameBase")==0)
			{
				model.getline(buffer, 255, '"');
				model.getline(buffer, 255, '"');
				models[currentmodel-1].animations[currentanimation-1].aninamebase = new char[strlen(buffer)+1];
				strcpy(models[currentmodel-1].animations[currentanimation-1].aninamebase,buffer);
			}
		} while (strcmp(buffer,"eof")!=0);
	}

	for(int i=0; i<totalModels; ++i)
	{
		
		for(int j=0; j<models[i].numAnimations; ++j)
		{
			for(int k=0; k<models[i].animations[j].numFrames; ++k)
			{
				loadername = new char[255];
				strcpy(loadername,models[i].filenamebase);
				strcat(loadername,models[i].animations[j].aninamebase);
				length = strlen(loadername);
				loadername[length]=k+49;
				loadername[length+1]='\0';
				strcat(loadername,".obj");
				cout << "Loading " << loadername << endl;
				loadFrame(models[i].animations[j].frames[k], loadername);
				models[i].animations[j].currentframe = 0;
				delete [] loadername;
			}
		}
		
	}
}

void loadEnemies(char *enemyname)
{
	char buffer[255];
	char *stop;
	
	ifstream enemy(enemyname);

	totalEnemies=0;
	int currentenemy=0;

	if (!enemy.is_open())
		cout << "Could not open enemy file";
	else
	{
		do
		{
			enemy.getline(buffer, 255, '"');
			enemy.getline(buffer, 255, '"');

			if(strcmp(buffer,"numEnemies")==0)
			{
				enemy.getline(buffer, 255, '"');
				enemy.getline(buffer, 255, '"');
				totalEnemies = strtol(buffer,&stop,10);
				enemies = new enemyStruct[totalEnemies];
			}

			if(strcmp(buffer,"enemy")==0)
			{
				currentenemy++;
				enemies[currentenemy-1].health=100;
				enemies[currentenemy-1].attackDamage=5;
				enemies[currentenemy-1].dying=false;
				enemies[currentenemy-1].currentanimation=1;
				enemy.getline(buffer, 255, '"');
				enemy.getline(buffer, 255, '"');
				for(int i=0; i<totalModels; ++i)
					if(strcmp(buffer,models[i].name)==0)
						enemies[currentenemy-1].modelIndex = i;
			}

			if(strcmp(buffer,"startlocation")==0)
			{
				enemy.getline(buffer, 255, '"');
				enemy.getline(buffer, 255, '"');
				enemies[currentenemy-1].position.X = strtod(buffer, &stop);
				enemy.getline(buffer, 255, '"');
				enemy.getline(buffer, 255, '"');
				enemies[currentenemy-1].position.Y = strtod(buffer, &stop);
				enemy.getline(buffer, 255, '"');
				enemy.getline(buffer, 255, '"');
				enemies[currentenemy-1].position.Z = strtod(buffer, &stop);
				enemies[currentenemy-1].state=0;
			}

			if(strcmp(buffer,"gravity")==0)
			{
				enemy.getline(buffer,255,'"');
				enemy.getline(buffer,255,'"');
				if(strcmp(buffer,"no")==0)
					enemies[currentenemy-1].gravity = false;
				else
					enemies[currentenemy-1].gravity = true;
			}

			if(strcmp(buffer,"clipping")==0)
			{
				enemy.getline(buffer,255,'"');
				enemy.getline(buffer,255,'"');
				if(strcmp(buffer,"no")==0)
					enemies[currentenemy-1].clipping = false;
				else
					enemies[currentenemy-1].clipping = true;
			}
	
			if(strcmp(buffer,"attackdistance")==0)
			{
				enemy.getline(buffer,255,'"');
				enemy.getline(buffer,255,'"');
				enemies[currentenemy-1].attackDistance = strtod(buffer, &stop);
			}

			if(strcmp(buffer,"speed")==0)
			{
				enemy.getline(buffer,255,'"');
				enemy.getline(buffer,255,'"');
				enemies[currentenemy-1].speed = strtod(buffer, &stop);
			}

			if(strcmp(buffer,"attackdamage")==0)
			{
				enemy.getline(buffer,255,'"');
				enemy.getline(buffer,255,'"');
				enemies[currentenemy-1].attackDamage = strtol(buffer, &stop, 10);
			}

			if(strcmp(buffer,"health")==0)
			{
				enemy.getline(buffer,255,'"');
				enemy.getline(buffer,255,'"');				
				enemies[currentenemy-1].health = strtol(buffer, &stop, 10);
			}

		} while (strcmp(buffer,"eof")!=0);
	}

}
void loadEntities(char *entname)
{
	char buffer[255];
	char *stop;
	ifstream ents(entname);

	if (!ents.is_open())
		cout << "Could not open entities file, global lighting enabled";
	else
	{
		ents.getline(buffer, 255, '\n');
		totalEntities = strtol(buffer,&stop,10);
		entities = new entityStruct[totalEntities];

		for (int i=0; i<totalEntities; ++i)
		{
			ents.getline(buffer, 32, '\n');
			entities[i].type = new char[strlen(buffer)+1];
			strcpy(entities[i].type,buffer);

			if(strcmp(entities[i].type,"PlayerStart")==0)
			{
				ents.getline(buffer, 255, ' ');
				entities[i].point.X = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].point.Y = strtod(buffer,&stop);
				ents.getline(buffer, 255, '\n');
				entities[i].point.Z = strtod(buffer,&stop);
			}
			if(strcmp(entities[i].type,"Light")==0)
			{
				ents.getline(buffer, 255, ' ');
				entities[i].color.R = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].color.G = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].color.B = strtod(buffer,&stop);
				ents.getline(buffer, 255, '\n');
				entities[i].color.A = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].ambientcolor.R = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].ambientcolor.G = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].ambientcolor.B = strtod(buffer,&stop);
				ents.getline(buffer, 255, '\n');
				entities[i].ambientcolor.A = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].point.X = strtod(buffer,&stop);
				ents.getline(buffer, 255, ' ');
				entities[i].point.Y = strtod(buffer,&stop);
				ents.getline(buffer, 255, '\n');
				entities[i].point.Z = strtod(buffer,&stop);
			}
			ents.getline(buffer, 255, ' ');
		}
	}
}
void loadMaterials(char *matname)
{
	char buffer[255];
	char *stop;
	int assignMaterial = 0;
	int assignGroups;
	int assignObjects = 0;
	int i,j,k;
	
	
	ifstream matfile(matname);

	if (!matfile.is_open())
		cout << "Could not open File";
	else
	{
		matfile.getline(buffer, 255, '\n');
		totalTextures = strtol(buffer,&stop,10);

		textures = new textureStruct[totalTextures];

		for (i=0; i<totalTextures; ++i)
		{
			if(i==totalTextures-1)
				matfile.getline(buffer, 255, '\n');
			else
				matfile.getline(buffer, 255, ' ');
			textures[i].name = new char[strlen(buffer)+1];
			strcpy(textures[i].name,buffer);
		}
		
		matfile.getline(buffer, 255, '\n');
		totalMaterials = strtol(buffer, &stop, 10);
		materials = new materialStruct[totalMaterials];

		for(i=0; i < totalMaterials; ++i)
		{
			matfile.getline(buffer, 255, '\n'); 
			if(strcmp(buffer,"default")==0)
			{
				matfile.getline(buffer, 255, ' '); defaultMaterial.ambient.R=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.ambient.G=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.ambient.B=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); defaultMaterial.ambient.A=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.diffuse.R=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.diffuse.G=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.diffuse.B=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); defaultMaterial.diffuse.A=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.specular.R=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.specular.G=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); defaultMaterial.specular.B=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); defaultMaterial.specular.A=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); defaultMaterial.shine=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); defaultMaterial.texture=strtol(buffer, &stop, 10);			
				strcpy(materials[i].name, "NULL");
			}
			else
			{				
				strcpy(materials[i].name,buffer);

				matfile.getline(buffer, 255, ' '); materials[i].ambient.R=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].ambient.G=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].ambient.B=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); materials[i].ambient.A=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].diffuse.R=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].diffuse.G=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].diffuse.B=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); materials[i].diffuse.A=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].specular.R=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].specular.G=strtod(buffer, &stop);
				matfile.getline(buffer, 255, ' '); materials[i].specular.B=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); materials[i].specular.A=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); materials[i].shine=strtod(buffer, &stop);
				matfile.getline(buffer, 255, '\n'); materials[i].texture=strtol(buffer, &stop, 10);
			}
		}

		for(i=0; i<totalObjects; ++i)
			objects[i].material = -1;

		matfile.getline(buffer, 255, '\n');
		assignGroups = strtol(buffer, &stop, 10);

		for (i=0; i<assignGroups; ++i)
		{		
			matfile.getline(buffer, 255, ' ');
			assignMaterial=-1;

			for(j=0; j<totalMaterials; ++j)
				if(strcmp(materials[j].name,buffer)==0)
					assignMaterial=j;

			matfile.getline(buffer, 255, '\n');

			assignObjects=strtol(buffer, &stop, 10);

			for(j=0;j<assignObjects;++j)
			{
				if(j==assignObjects-1)
					matfile.getline(buffer, 255, '\n');
				else
					matfile.getline(buffer, 255, ' ');
				
				for(k=0; k<totalObjects; ++k)
					if(strcmp(objects[k].name,buffer)==0)
						objects[k].material=assignMaterial;
			}
		}
	}
}
void getLevelInfo(char *levelname)
{
	char buffer[255];
	int size = 0;
	int position = 0;
	int state = 0;
	
	ifstream level(levelname);

	if (!level.is_open())
		cout << "Could not open File";
	else
	{
		level.seekg (0, ios::end);
		size = level.tellg();
		level.seekg (0, ios::beg);
		do
		{
			level.getline(buffer, 255, '\n');
			switch(buffer[0])
			{
			case 'v':
				state=0;
				switch(buffer[1])
				{
				case 't':
					totalTexCoords++;
					break;
				case 'n':
					totalNormals++;
					break;
				case ' ':
					totalVertices++;
					break;
				}
				state=0;
				break;
			case 'f':
				if(state==0)
					totalObjects++;
				state=1;
				totalFaces++;
				break;
			default:				
				break;
			}
		position = level.tellg();
		} while(position < size);
	}
}
void initializeAll()
{
	viewMode = 1;
	lighting = true;
	physics = true;
	clipping = true;

	totalVertices=0;
	totalNormals=0;
	totalTexCoords=0;
	totalFaces=0;
	totalObjects=0;
	totalEntities = 0;
	totalMaterials = 0;
	totalTextures = 0;
	totalModels = 0;

	vertices = new pointStruct[totalVertices];
	normals = new pointStruct[totalNormals];
	texCoords = new texCoordStruct[totalTexCoords];
	faces = new faceStruct[totalFaces];
	objects = new objectStruct[totalObjects];
	entities = new entityStruct[totalEntities];
	materials = new materialStruct[totalMaterials];
	textures = new textureStruct[totalTextures];
	models = new modelStruct[totalModels];
}

void initializeSounds()
{
	ALboolean loop;
	ALfloat sourceP[3];
	ALfloat sourceV[3];
	int i;

	alListenerfv(AL_POSITION,SOUNDlistenerP);
	alListenerfv(AL_VELOCITY,SOUNDlistenerV);
	alListenerfv(AL_ORIENTATION,SOUNDlistenerO);

	alGenBuffers(NUM_SOUND_BUFFERS,&SOUNDbuffer[0]);

	alutLoadWAVFile("Death.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[0],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("Jetpack.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[1],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("Enemygrumble.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[2],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("Enemysighting.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[3],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("Enemyhunting.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[4],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("Enemyattack.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[5],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("Gunfire.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[6],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("PlayerDamage.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[7],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alutLoadWAVFile("EnemyHurt.wav",&SOUNDformat,&SOUNDdata,&SOUNDsize,&SOUNDfreq,&loop);
	alBufferData(SOUNDbuffer[8],SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);
	alutUnloadWAV(SOUNDformat,SOUNDdata,SOUNDsize,SOUNDfreq);

	alGenSources(NUM_SOUND_SOURCES+totalEnemies,&SOUNDsource[0]);

	//gun
	alSourcef(SOUNDsource[0],AL_PITCH,1.0f);
	alSourcef(SOUNDsource[0],AL_GAIN,1.0f);
	alSourcefv(SOUNDsource[0],AL_POSITION,SOUNDlistenerP);
	alSourcefv(SOUNDsource[0],AL_VELOCITY,SOUNDlistenerV);
	alSourcei(SOUNDsource[0],AL_BUFFER,SOUNDbuffer[6]);
	alSourcei(SOUNDsource[0],AL_LOOPING,AL_FALSE);

	//jetpack
	alSourcef(SOUNDsource[1],AL_PITCH,1.0f);
	alSourcef(SOUNDsource[1],AL_GAIN,1.0f);
	alSourcefv(SOUNDsource[1],AL_POSITION,SOUNDlistenerP);
	alSourcefv(SOUNDsource[1],AL_VELOCITY,SOUNDlistenerV);
	alSourcei(SOUNDsource[1],AL_BUFFER,SOUNDbuffer[1]);
	alSourcei(SOUNDsource[1],AL_LOOPING,AL_TRUE);

	//player
	alSourcef(SOUNDsource[2],AL_PITCH,1.0f);
	alSourcef(SOUNDsource[2],AL_GAIN,1.0f);
	alSourcefv(SOUNDsource[2],AL_POSITION,SOUNDlistenerP);
	alSourcefv(SOUNDsource[2],AL_VELOCITY,SOUNDlistenerV);
	alSourcei(SOUNDsource[2],AL_BUFFER,SOUNDbuffer[7]);
	alSourcei(SOUNDsource[2],AL_LOOPING,AL_FALSE);

	//enemy hurt sounds
	alSourcef(SOUNDsource[3],AL_PITCH,1.0f);
	alSourcef(SOUNDsource[3],AL_GAIN,1.0f);
	alSourcefv(SOUNDsource[3],AL_POSITION,SOUNDlistenerP);
	alSourcefv(SOUNDsource[3],AL_VELOCITY,SOUNDlistenerV);
	alSourcei(SOUNDsource[3],AL_BUFFER,SOUNDbuffer[8]);
	alSourcei(SOUNDsource[3],AL_LOOPING,AL_FALSE);

	//enemy

	for(i=NUM_SOUND_SOURCES;i<NUM_SOUND_SOURCES+totalEnemies;++i)
	{
		sourceP[0]=enemies[i-NUM_SOUND_SOURCES].position.X;
		sourceP[1]=enemies[i-NUM_SOUND_SOURCES].position.Y;
		sourceP[2]=enemies[i-NUM_SOUND_SOURCES].position.Z;

		sourceV[0]=0;
		sourceV[1]=0;
		sourceV[2]=0;

		alSourcef(SOUNDsource[i],AL_PITCH,1.0f);
		alSourcef(SOUNDsource[i],AL_GAIN,1.0f);
		alSourcefv(SOUNDsource[i],AL_POSITION,sourceP);
		alSourcefv(SOUNDsource[i],AL_VELOCITY,sourceV);
		alSourcei(SOUNDsource[i],AL_BUFFER,SOUNDbuffer[2]);
		alSourcei(SOUNDsource[i],AL_LOOPING,AL_TRUE);
		if(Audio) alSourcePlay(SOUNDsource[i]);
	}
}

void loadAll(char *allname)
{
	char levelname[255];
	char entname[255];
	char matname[255];
	char modname[255];
	char enmname[255];
	int i;

	strcpy(levelname, allname);
	strcpy(entname, allname);
	strcpy(matname, allname);
	strcpy(modname, allname);
	strcpy(enmname, allname);

	strcat(levelname, ".obj");
	strcat(entname, ".ent");
	strcat(matname, ".mat");
	strcat(modname, ".mod");
	strcat(enmname, ".enm");

	toConsole("Fetching level info...");
	getLevelInfo(levelname);
	toConsoleCat(" done!");
	
	vertices = new pointStruct[totalVertices];
	normals = new pointStruct[totalNormals];
	texCoords = new texCoordStruct[totalTexCoords];
	faces = new faceStruct[totalFaces];
	objects = new objectStruct[totalObjects];

	toConsole("Parsing level geometry...");
	loadLevel(levelname);
	toConsoleCat(" done!");
	glutPostRedisplay();

	toConsole("Loading Entities and Lightsources...");
	loadEntities(entname);
	toConsoleCat(" done!");

	toConsole("Loading Materials definitions...");
	loadMaterials(matname);
	toConsoleCat(" done!");

	loadModel(modname);
	loadEnemies(enmname);

	toConsoleCat(" done!");

	for(i=0;i<totalEntities;++i)
	{
		if(strcmp(entities[i].type,"PlayerStart")==0)
		{
			playerPosition.X=entities[i].point.X;
			playerPosition.Y=entities[i].point.Y;
			playerPosition.Z=entities[i].point.Z;
			playerView.X=entities[i].point.X;
			playerView.Y=entities[i].point.Y;
			playerView.Z=entities[i].point.Z+0.5;
			playerUp.X=0;
			playerUp.Y=1;
			playerUp.Z=0;
		}
		else if(strcmp(entities[i].type,"Light")==0)
		{
			GLfloat light_position[] = {entities[i].point.X, entities[i].point.Y, entities[i].point.Z, 1.0};
			GLfloat light_maincolor[] = {entities[i].color.R, entities[i].color.G, entities[i].color.B, entities[i].color.A};
			GLfloat light_ambientcolor[] = {entities[i].ambientcolor.R, entities[i].ambientcolor.G, entities[i].ambientcolor.B, entities[i].ambientcolor.A};

			glLightfv(GL_LIGHT0+i, GL_POSITION, light_position);
			glLightfv(GL_LIGHT0+i, GL_DIFFUSE, light_maincolor);
			glLightfv(GL_LIGHT0+i, GL_SPECULAR, light_maincolor);
			glLightfv(GL_LIGHT0+i, GL_AMBIENT, light_ambientcolor);

			glEnable(GL_LIGHT0+i);
		}
	}
	GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	for(i=0; i<totalTextures; ++i)
	{
		loadTexture(i);
	}

	glGenTextures(totalTextures+1, &texture[0]);
	for(i=0; i<totalTextures; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textures[i].size, textures[i].size, GL_RGB, GL_UNSIGNED_BYTE, textures[i].data);
	}
	glEnable(GL_TEXTURE_2D);

	Live();

	initializeSounds();
	dumpLevelInfo();

}

void clearAll()
{
	int i,j,k,l;
	
	cout << "totalFaces =" << totalFaces << endl;
	for(i=0;i<totalFaces;++i)
	{
		delete [] faces[i].vertIndex;
		delete [] faces[i].normIndex;
		delete [] faces[i].texIndex;
	}
	delete [] faces;

	cout << "totalObjects =" << totalObjects << endl;
	for(i=0;i<totalObjects;++i)
		delete [] objects[i].name;
	delete [] objects;

	cout << "totalEntities =" << totalEntities << endl;
	for(i=0;i<totalEntities;++i)
		delete [] entities[i].type;
	delete [] entities;
	
	cout << "totalMaterials =" << totalMaterials << endl;
//	for(i=0;i<totalMaterials;++i)
//			delete [] materials[i].name;
	delete [] materials;

	cout << "totalTextures =" << totalTextures << endl;
	for(i=0;i<totalTextures;++i)
	{
		delete [] textures[i].name;
		delete [] textures[i].data;
	}
	delete [] textures;

	cout << "totalModels =" << totalModels << endl;
	for(i=0;i<totalModels;++i)
	{
		delete [] models[i].name;
		delete [] models[i].filenamebase;
		for(j=0; j<models[i].numAnimations; ++j)
		{
			delete [] models[i].animations[j].aninamebase;
			delete [] models[i].animations[j].name;
			for(k=0; k<models[i].animations[j].numFrames; ++k)
			{
				delete [] models[i].animations[j].frames[k].vertices;
				delete [] models[i].animations[j].frames[k].normals;
				delete [] models[i].animations[j].frames[k].texCoords;
				for(l=0; l<models[i].animations[j].frames[k].totalFaces; ++l)
				{
					delete [] models[i].animations[j].frames[k].faces[l].vertIndex;
					delete [] models[i].animations[j].frames[k].faces[l].normIndex;
					delete [] models[i].animations[j].frames[k].faces[l].texIndex;
				}
			}
		}
	}
	if(totalModels>0)
		delete [] models;

	if(totalEnemies>0)
		delete [] enemies;


	cout << "totalVertices =" << totalVertices << endl;
	delete [] vertices; 
	cout << "totalNormals =" << totalNormals << endl;
	delete [] normals; 
	cout << "totalTexCoords =" << totalTexCoords << endl;
	delete [] texCoords; 

	totalVertices=0;
	totalNormals=0;
	totalTexCoords=0;
	totalFaces=0;
	totalObjects=0;
	totalEntities = 0;
	totalMaterials = 0;
	totalTextures = 0;
	totalModels = 0;
	totalEnemies = 0;
}

void processMenuScript(char *command)
{
	int i;
	char output[255];
	char verb[80];
	char noun[80];
	bool foundVerbNoun = false;

	for(i=0; i<strlen(command); ++i)
	{
		if(command[i]==' ')
		{			
			strncpy(verb,command,i);
			foundVerbNoun = true;
			verb[i]='\0';

			for(unsigned int j=0; j<strlen(command)-i; ++j)
			{
				noun[j]=command[i+j+1];
			}
			noun[strlen(command)-i]='\0';
		}
	}

	if(foundVerbNoun==false)
		strcpy(verb,command);


	if(strcmp(verb,"showRes")==0)
	{
		strcpy(output,screenSettings[screenResIndex]);
		
	}
	else if(strcmp(verb,"showFSStatus")==0)
	{
		if(fullscreen==true)
			strcpy(output,"true");
		else
			strcpy(output,"false");
	}
	else if(strcmp(verb, "showkey")==0)
	{
		if(strcmp(noun, "walkup")==0)
			output[0]=keys.walkUp; 
		else if(strcmp(noun, "walkdown")==0)
			output[0]=keys.walkDown;
		else if(strcmp(noun, "strafel")==0)
			output[0]=keys.strafeL;
		else if(strcmp(noun, "strafer")==0)
			output[0]=keys.strafeR;
		else if(strcmp(noun, "jump")==0)
			output[0]=keys.jump;
		else if(strcmp(noun, "cameramode")==0)
			output[0]=keys.cameraMode;
		else if(strcmp(noun, "console")==0)
			output[0]=keys.console;
		else if(strcmp(noun, "menu")==0)
			output[0]=keys.menu;
		output[1]='\0';
		
		if(output[0]==27)
			strcpy(output,"Esc");
		if(output[0]==' ')
			strcpy(output,"Spacebar");
	}
	else if(strcmp(verb,"showSoundStatus")==0)
	{
		if(Audio)
			strcpy(output,"On");
		else
			strcpy(output,"Off");
	}
	else
		strcpy(output,"Bad menuScript Command");

	for(i=0; i<strlen(output); ++i)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, output[i]);
}

void displayMenu(int menuIndex)
{
	int tabWidth = 0;
	bool hovering;
	int i,j;
	int backupOffset=0;

	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, actualX, 0, actualY);
	glScalef(1, -1, 1);
	glTranslatef(0, -actualY, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3d(0.0,0.0,0.2);
	glBegin(GL_QUADS);
		glVertex2d(actualX/2-menuBGwidth/2,actualY/2-menuBGheight/2);
		glVertex2d(actualX/2+menuBGwidth/2,actualY/2-menuBGheight/2);
		glVertex2d(actualX/2+menuBGwidth/2,actualY/2+menuBGheight/2);
		glVertex2d(actualX/2-menuBGwidth/2,actualY/2+menuBGheight/2);
	glEnd();

	for(i=0; i<strlen(menus[menuIndex].text); ++i)
		tabWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, menus[menuIndex].text[i]);

	glColor3d(0.0,0.0,0.3);
	glBegin(GL_QUADS);
		glVertex2d(actualX/2-menuBGwidth/2,actualY/2-(menuBGheight/2+menuHeadingOffset+menuHeadingSize));
		glVertex2d(actualX/2-menuBGwidth/2+tabWidth+menuHeadingOffset,actualY/2-(menuBGheight/2+menuHeadingOffset+menuHeadingSize));
		glVertex2d(actualX/2-menuBGwidth/2+tabWidth+menuHeadingOffset,actualY/2-menuBGheight/2);
		glVertex2d(actualX/2-menuBGwidth/2,actualY/2-menuBGheight/2);
	glEnd();

	glColor3d(1.0,1.0,1.0);
	
	glRasterPos2f(actualX/2-(menuBGwidth/2)+(menuHeadingOffset/2),actualY/2-menuBGheight/2-menuHeadingSize/2);
	for(i=0; i<strlen(menus[menuIndex].text); ++i)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menus[menuIndex].text[i]);

	hovering = false;
	for(i=0; i<menus[menuIndex].numMenuItems; ++i)
	{
		
		if(strcmp(menus[menuIndex].menuItems[i].text,"runNL")==0)
		{
			glColor3d(1,1,1);
			GLdouble sx = actualX/2-menuBGwidth/2+10;
			GLdouble sy = actualY/2-(menuBGheight/2-menuItemSize-menuItemOffset)+(i*(menuItemOffset*2));
			
			glRasterPos2f(sx,sy);
			processMenuScript(menus[menuIndex].menuItems[i].command);
		}
		else if(strcmp(menus[menuIndex].menuItems[i].text,"run")==0)
		{
			glColor3d(1,1,1);
			processMenuScript(menus[menuIndex].menuItems[i].command);
			backupOffset++;
		}
		else
		{
			GLdouble sx = actualX/2-menuBGwidth/2+10;
			GLdouble sy = actualY/2-(menuBGheight/2-menuItemSize-menuItemOffset)+((i-backupOffset)*(menuItemOffset*2));
			GLdouble sx2 = 0;
			
			if(menus[menuIndex].fixedfont)
				for(j=0; j<strlen(menus[menuIndex].menuItems[i].text); ++j)
					sx2 += glutBitmapWidth(GLUT_BITMAP_9_BY_15, menus[menuIndex].menuItems[i].text[j]);
			else
				for(j=0; j<strlen(menus[menuIndex].menuItems[i].text); ++j)
					sx2 += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, menus[menuIndex].menuItems[i].text[j]);

			if( mouseX>sx &&
				mouseX<sx+sx2 &&
				mouseY<sy &&
				mouseY>sy-menuItemSize)
			{
				glColor3d(1,1,0);
				menuHover = i;
				hovering = true;
			}
			else
			{
				glColor3d(1,1,1);
			}

			glRasterPos2f(sx,sy);

			if(menus[menuIndex].fixedfont)
				for(j=0; j<strlen(menus[menuIndex].menuItems[i].text); ++j)
					glutBitmapCharacter(GLUT_BITMAP_9_BY_15, menus[menuIndex].menuItems[i].text[j]);
			else
				for(j=0; j<strlen(menus[menuIndex].menuItems[i].text); ++j)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menus[menuIndex].menuItems[i].text[j]);
		}
	}
	if(hovering==false)
		menuHover = -1;

	if(getNextKeypress)
	{
		char prompt[80];
		strcpy(prompt,"Press a Key!");
		int pwidth = 0;

		glColor3d(0.4,0.05,0.05);
		glBegin(GL_QUADS);
			glVertex2d(actualX/2-100,actualY/2-20);
			glVertex2d(actualX/2+100,actualY/2-20);
			glVertex2d(actualX/2+100,actualY/2+20);
			glVertex2d(actualX/2-100,actualY/2+20);
		glEnd();

		for(i=0; i<strlen(prompt); ++i)
			pwidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, prompt[i]);

		glColor3d(1.0,1.0,1.0);
		glRasterPos2f(actualX/2-pwidth/2,actualY/2+4);
		
		for(i=0; i<strlen(prompt); ++i)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, prompt[i]);
		
	}
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);	
}
void setmaterial(int matNum)
{
	glBindTexture(GL_TEXTURE_2D, texture[materials[matNum].texture]);
	GLfloat ambientcolor[] = {  materials[matNum].ambient.R,
								materials[matNum].ambient.G,
								materials[matNum].ambient.B,
								materials[matNum].ambient.A};
	GLfloat diffusecolor[] = {  materials[matNum].diffuse.R,
								materials[matNum].diffuse.G,
								materials[matNum].diffuse.B,
								materials[matNum].diffuse.A};
	GLfloat specularcolor[] = { materials[matNum].specular.R,
								materials[matNum].specular.G,
								materials[matNum].specular.B,
								materials[matNum].specular.A};
	GLfloat shininess[] = {		materials[matNum].shine};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientcolor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffusecolor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularcolor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void setDefaultMaterial()
{
	glBindTexture(GL_TEXTURE_2D, texture[defaultMaterial.texture]);
	GLfloat ambientcolor[] = {  defaultMaterial.ambient.R,
								defaultMaterial.ambient.G,
								defaultMaterial.ambient.B,
								defaultMaterial.ambient.A};
	GLfloat diffusecolor[] = {  defaultMaterial.diffuse.R,
								defaultMaterial.diffuse.G,
								defaultMaterial.diffuse.B,
								defaultMaterial.diffuse.A};
	GLfloat specularcolor[] = { defaultMaterial.specular.R,
								defaultMaterial.specular.G,
								defaultMaterial.specular.B,
								defaultMaterial.specular.A};
	GLfloat shininess[] = {		defaultMaterial.shine};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientcolor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffusecolor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularcolor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}


pointStruct getCross(pointStruct vector1, pointStruct vector2)
{
	pointStruct normal;
	normal.X = ((vector1.Y * vector2.Z) - (vector1.Z * vector2.Y));
	normal.Y = ((vector1.Z * vector2.X) - (vector1.X * vector2.Z));
	normal.Z = ((vector1.X * vector2.Y) - (vector1.Y * vector2.X));
	return normal;
}

GLdouble getDot(pointStruct vector1, pointStruct vector2) 
{
	return ((vector1.X * vector2.X) + (vector1.Y * vector2.Y) + (vector1.Z * vector2.Z));
}

GLdouble getMagnitude(pointStruct normal)
{
	return (GLdouble)sqrt((normal.X * normal.X) + (normal.Y * normal.Y) + (normal.Z * normal.Z));
}

pointStruct Normalize(pointStruct normal)
{
	GLdouble magnitude = getMagnitude(normal);
	normal.X /= magnitude;
	normal.Y /= magnitude;
	normal.Z /= magnitude;
	return normal;
}


pointStruct getNormal(faceStruct face)					
{
	pointStruct vector1, vector2;
	vector1.X = vertices[face.vertIndex[2]].X - vertices[face.vertIndex[0]].X;
	vector1.Y = vertices[face.vertIndex[2]].Y - vertices[face.vertIndex[0]].Y;
	vector1.Z = vertices[face.vertIndex[2]].Z - vertices[face.vertIndex[0]].Z;

	vector2.X = vertices[face.vertIndex[1]].X - vertices[face.vertIndex[0]].X;
	vector2.Y = vertices[face.vertIndex[1]].Y - vertices[face.vertIndex[0]].Y;
	vector2.Z = vertices[face.vertIndex[1]].Z - vertices[face.vertIndex[0]].Z;

	pointStruct normal = getCross(vector1, vector2);
	normal = Normalize(normal);

	return normal;
}

GLdouble getPlaneDistance(pointStruct normal, pointStruct point)
{	
	float distance = 0;
	distance = -((normal.X * point.X) + (normal.Y * point.Y) + (normal.Z * point.Z));

	return distance;
}

GLdouble Distance(pointStruct point1, pointStruct point2)
{
	GLdouble distance = sqrt((point2.X - point1.X) * (point2.X - point1.X) +
						    (point2.Y - point1.Y) * (point2.Y - point1.Y) +
						    (point2.Z - point1.Z) * (point2.Z - point1.Z));

	return (GLdouble)distance;
}

GLdouble angleBetweenVectors(pointStruct vector1, pointStruct vector2)
{							
	float dotProduct = getDot(vector1, vector2);				
	float vectorsMagnitude = getMagnitude(vector1) * getMagnitude(vector2);

	double angle = acos( dotProduct / vectorsMagnitude );
	//if(_isnan(angle))
	//	return 0;
	return(angle);
}


bool InsidePolygon(pointStruct intersection, faceStruct face)
{
	GLdouble Angle = 0.0;
	pointStruct v1, v2;

	if(DRAW_COLLISION_POINTS)
	{
		glPushMatrix();
		glTranslated(intersection.X,intersection.Y,intersection.Z);
		glutSolidSphere(0.05,10,10);
		glPopMatrix();
	}
	
	for (int i = 0; i < face.numPoints; ++i)
	{	
		v1.X = vertices[face.vertIndex[i]].X - intersection.X;
		v1.Y = vertices[face.vertIndex[i]].Y - intersection.Y;
		v1.Z = vertices[face.vertIndex[i]].Z - intersection.Z;

		v2.X = vertices[face.vertIndex[(i+1)%face.numPoints]].X - intersection.X;
		v2.Y = vertices[face.vertIndex[(i+1)%face.numPoints]].Y - intersection.Y;
		v2.Z = vertices[face.vertIndex[(i+1)%face.numPoints]].Z - intersection.Z;
												
		Angle += angleBetweenVectors(v1, v2);
	}
											
	if(Angle > 6.28318)
		return true;
		
	return false;
}

int classifySphere(pointStruct &center, pointStruct &normal, pointStruct &point, GLdouble radius, GLdouble &distance)
{
	GLdouble d = (GLdouble)getPlaneDistance(normal, point);
	distance = (normal.X * center.X + normal.Y * center.Y + normal.Z * center.Z + d);

	if(absolute(distance) < radius)
		return INTERSECTS;
	else if(distance >= radius)
		return FRONT;
	return BEHIND;
}

pointStruct getCollisionOffset(pointStruct &normal, GLdouble radius, GLdouble distance)
{
	pointStruct offset;
	offset.X = 0.0;
	offset.Y = 0.0;
	offset.Z = 0.0;

	if(distance > 0.0)
	{
		float distanceOver = radius - distance;
		offset.X = normal.X * distanceOver;
		offset.Y = normal.Y * distanceOver;
		offset.Z = normal.Z * distanceOver;
	}
	else
	{
		float distanceOver = radius + distance;
		offset.X = normal.X * -distanceOver;
		offset.Y = normal.Y * -distanceOver;
		offset.Z = normal.Z * -distanceOver;
	}

	return offset;
}



pointStruct ClosestPointOnLine(pointStruct v1, pointStruct v2, pointStruct point)
{
	pointStruct vector1;
    pointStruct vector2;
    pointStruct vector3;

	vector1.X = point.X - v1.X;
	vector1.Y = point.Y - v1.Y;
	vector1.Z = point.Z - v1.Z;

	vector2.X = v2.X - v1.X;
	vector2.Y = v2.Y - v1.Y;
	vector2.Z = v2.Z - v1.Z;
	
	vector2 = Normalize(vector2);

    GLdouble d = Distance(v1, v2);

    GLdouble t = getDot(vector2, vector1);

	if (t <= 0) 
		return v1;
    if (t >= d) 
		return v2;
 
	vector3.X = vector2.X * t;
	vector3.Y = vector2.Y * t;
	vector3.Z = vector2.Z * t;

	pointStruct closestPoint;
	
	closestPoint.X = v1.X + vector3.X;
	closestPoint.Y = v1.Y + vector3.Y;
	closestPoint.Z = v1.Z + vector3.Z;

	return closestPoint;
}

bool edgeSphereCollision(pointStruct &center, faceStruct face, GLdouble radius)
{
	pointStruct point;

	for(int i = 0; i < face.numPoints; i++)
	{
		point = ClosestPointOnLine(vertices[face.vertIndex[i]], vertices[face.vertIndex[(i + 1) % face.numPoints]], playerPosition);
		
		if(DRAW_COLLISION_POINTS)
		{
			glPushMatrix();
				glTranslated(point.X,point.Y,point.Z);
				glutSolidSphere(0.05,10,10);
			glPopMatrix();
		}

		GLdouble distance = Distance(point, playerPosition);

		if(distance < radius)
			return true;
	}

	return false;
}

void checkCollision(pointStruct &position, bool isplayer, GLdouble radius)
{
	for(int i = 0; i < totalFaces; ++i)
	{	
		pointStruct normal = getNormal(faces[i]);
		GLdouble distance = 0.0;

		int classification = classifySphere(position, normal, vertices[faces[i].vertIndex[0]], radius, distance);
		if(classification == INTERSECTS)
		{					
			faces[i].visible = true;
			pointStruct offset;

			offset.X = normal.X * distance;
			offset.Y = normal.Y * distance;
			offset.Z = normal.Z * distance;

			pointStruct intersection;

			intersection.X = position.X - offset.X;
			intersection.Y = position.Y - offset.Y;
			intersection.Z = position.Z - offset.Z;
			
			if(InsidePolygon(intersection, faces[i]))			
			{
				offset = getCollisionOffset(normal, radius, distance);

				position.X += offset.X;
				position.Y += offset.Y;
				position.Z += offset.Z;

				if(isplayer==true)
				{
					playerView.X += offset.X;
					playerView.Y += offset.Y;
					playerView.Z += offset.Z;
				}

				faces[i].collision = true;
			}
			if(edgeSphereCollision(position, faces[i], radius/2))
			{
				offset = getCollisionOffset(normal, radius/2, distance);

				position.X += offset.X;
				position.Y += offset.Y;
				position.Z += offset.Z;

				if(isplayer==true)
				{
					playerView.X += offset.X;
					playerView.Y += offset.Y;
					playerView.Z += offset.Z;
				}

				faces[i].collision = true;
			}
		}
		else if(classification == BEHIND)
		{
			faces[i].visible = true;
		}
		else
			faces[i].visible = false;
	}
}

void movePlayer()
{
	pointStruct vector;
	vector.X = playerView.X - playerPosition.X;
	vector.Y = playerView.Y - playerPosition.Y;
	vector.Z = playerView.Z - playerPosition.Z;
	
	vector=Normalize(vector);

	playerPosition.X += vector.X * currentSpeed;
	playerPosition.Y += vector.Y * currentSpeed;
	playerPosition.Z += vector.Z * currentSpeed;
	playerView.X += vector.X * currentSpeed;
	playerView.Y += vector.Y * currentSpeed;
	playerView.Z += vector.Z * currentSpeed;
}

void moveEnemy()
{
	pointStruct vector;

	for(int i=0; i<totalEnemies; ++i)
	{
		if(enemies[i].dying==false && enemies[i].health>0)
		{
			GLdouble distance;
			distance = Distance(enemies[i].position,playerPosition);

			if(distance < enemies[i].attackDistance)
			{
				if(enemies[i].state==0)
				{
					if(Audio)
					{
						alSourceStop(SOUNDsource[NUM_SOUND_SOURCES+i]);
						alSourcei(SOUNDsource[NUM_SOUND_SOURCES+i],AL_BUFFER,SOUNDbuffer[3]);
						alSourcePlay(SOUNDsource[NUM_SOUND_SOURCES+i]);
					}
					enemies[i].state=1;
				}
				else if (enemies[i].state==1)
				{
					if(Audio)
					{
						alSourceStop(SOUNDsource[NUM_SOUND_SOURCES+i]);
						alSourcei(SOUNDsource[NUM_SOUND_SOURCES+i],AL_BUFFER,SOUNDbuffer[4]);				
						alSourcePlay(SOUNDsource[NUM_SOUND_SOURCES+i]);
					}
					enemies[i].state=2;
				}
				
				enemies[i].currentanimation = 1;

				vector.X = playerPosition.X - enemies[i].position.X;
				vector.Y = playerPosition.Y - enemies[i].position.Y;
				vector.Z = playerPosition.Z - enemies[i].position.Z;
				vector = Normalize(vector);

				enemies[i].position.X += vector.X*enemies[i].speed;
				enemies[i].position.Y += vector.Y*enemies[i].speed;
				enemies[i].position.Z += vector.Z*enemies[i].speed;

				if(distance < models[enemies[i].modelIndex].radius)
				{
					if(playerRecoverTime==0)
					{
						playerRecoverTime=50;
						if(playerHealth>enemies[i].attackDamage)
						{						
							playerHealth-=enemies[i].attackDamage;						
							if(Audio)
								alSourcePlay(SOUNDsource[2]);
						}
						else if(!playerDead)
							Die();
					}				

					enemies[i].position.X -= vector.X*(models[enemies[i].modelIndex].radius-distance);
					enemies[i].position.X -= vector.Y*(models[enemies[i].modelIndex].radius-distance);
					enemies[i].position.X -= vector.Z*(models[enemies[i].modelIndex].radius-distance);
				}				
			}
			else
			{
				if(enemies[i].state!=0 && Audio)
				{
					alSourceStop(SOUNDsource[NUM_SOUND_SOURCES+i]);
					alSourcei(SOUNDsource[NUM_SOUND_SOURCES+i],AL_BUFFER,SOUNDbuffer[2]);
					alSourcePlay(SOUNDsource[NUM_SOUND_SOURCES+i]);
				}

				enemies[i].currentanimation = 0;
				enemies[i].state = 0;
			}

			if(enemies[i].gravity)
				enemies[i].position.Y-=gravity;
				
			if(enemies[i].clipping)
				checkCollision(enemies[i].position, false, models[enemies[i].modelIndex].radius);
		}
		else
			enemies[i].currentanimation = 2;

		enemies[i].rotation = angleBetweenVectors(enemies[i].position,playerPosition)*180/PI;
	}
}
void strafePlayer()
{
	pointStruct cross;
	pointStruct vector;
	vector.X = playerView.X - playerPosition.X;
	vector.Y = playerView.Y - playerPosition.Y;
	vector.Z = playerView.Z - playerPosition.Z;

	cross = getCross(vector, playerUp);
	cross = Normalize(cross);

	playerPosition.X += cross.X * currentSpeed;
	playerPosition.Z += cross.Z * currentSpeed;

	playerView.X += cross.X * currentSpeed;
	playerView.Z += cross.Z * currentSpeed;
}

void rotatePlayer(float angle, float x, float y, float z)
{
	pointStruct newPlayerView;

	pointStruct vector;
	vector.X = playerView.X - playerPosition.X;
	vector.Y = playerView.Y - playerPosition.Y;
	vector.Z = playerView.Z - playerPosition.Z;	

	playerRotation+=(angle*y)*180/PI;

	float cosTheta = (float)cos(angle);
	float sinTheta = (float)sin(angle);

	newPlayerView.X = (cosTheta + (1 - cosTheta) * x * x) * vector.X;
	newPlayerView.X += ((1 - cosTheta) * x * y - z * sinTheta) * vector.Y;
	newPlayerView.X += ((1 - cosTheta) * x * z + y * sinTheta) * vector.Z;

	newPlayerView.Y = ((1 - cosTheta) * x * y + z * sinTheta) * vector.X;
	newPlayerView.Y += (cosTheta + (1 - cosTheta) * y * y) * vector.Y;
	newPlayerView.Y += ((1 - cosTheta) * y * z - x * sinTheta) * vector.Z;

	newPlayerView.Z  = ((1 - cosTheta) * x * z - y * sinTheta) * vector.X;
	newPlayerView.Z += ((1 - cosTheta) * y * z + x * sinTheta) * vector.Y;
	newPlayerView.Z += (cosTheta + (1 - cosTheta) * z * z) * vector.Z;

	playerView.X = playerPosition.X + newPlayerView.X;
	playerView.Y = playerPosition.Y + newPlayerView.Y;
	playerView.Z = playerPosition.Z + newPlayerView.Z;
}

void drawModel(frameStruct &frame)
{
	for(int i=0; i<frame.totalFaces; ++i)
	{
		glBegin(GL_POLYGON);
		for(int k=0; k<frame.faces[i].numPoints; ++k)
		{					
			glNormal3d(	frame.normals[frame.faces[i].normIndex[k]].X,
						frame.normals[frame.faces[i].normIndex[k]].Y,
						frame.normals[frame.faces[i].normIndex[k]].Z);
			glTexCoord2d(	frame.texCoords[frame.faces[i].texIndex[k]].X,
							frame.texCoords[frame.faces[i].texIndex[k]].Y);
			glVertex3d( frame.vertices[frame.faces[i].vertIndex[k]].X,
						frame.vertices[frame.faces[i].vertIndex[k]].Y,
						frame.vertices[frame.faces[i].vertIndex[k]].Z);
		}
			glEnd();
	}
}
void display()
{
	int facesdrawn = 0;
	int i,k,o;
	int currentanimation = 0;
	int currentframe = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(lighting)
		glEnable(GL_LIGHTING);	
	else
		glDisable(GL_LIGHTING);

	switch(viewMode)
	{
	case 1:
		/*
		if(totalModels>0)
		{
			for(int i=0; i<models[playerModelIndex].totalFaces; ++i)
			{
				glBegin(GL_POLYGON);
				for(int k=0; k<models[playerModelIndex].faces[i].numPoints; ++k)
				{					
					glNormal3d(	models[playerModelIndex].normals[models[playerModelIndex].faces[i].normIndex[k]].X,
								models[playerModelIndex].normals[models[playerModelIndex].faces[i].normIndex[k]].Y,
								models[playerModelIndex].normals[models[playerModelIndex].faces[i].normIndex[k]].Z);
					glTexCoord2d(	models[playerModelIndex].texCoords[models[playerModelIndex].faces[i].texIndex[k]].X,
									models[playerModelIndex].texCoords[models[playerModelIndex].faces[i].texIndex[k]].Y);
					glVertex3d( models[playerModelIndex].vertices[models[playerModelIndex].faces[i].vertIndex[k]].X,
								models[playerModelIndex].vertices[models[playerModelIndex].faces[i].vertIndex[k]].Y,
								models[playerModelIndex].vertices[models[playerModelIndex].faces[i].vertIndex[k]].Z);
				}
				glEnd();
			}
		}*/
		gluLookAt(playerPosition.X,	playerPosition.Y,	playerPosition.Z,
			  playerView.X,		playerView.Y,		playerView.Z,
			  playerUp.X,		playerUp.Y,			playerUp.Z);
		break;
	case 2:
		
		glTranslated(0,0,-5);
		gluLookAt(playerPosition.X,	playerPosition.Y,	playerPosition.Z,
			  playerView.X,		playerView.Y,		playerView.Z,
			  playerUp.X,		playerUp.Y,			playerUp.Z);

		glPushMatrix();
						
			glTranslated(playerPosition.X,playerPosition.Y,playerPosition.Z);
			glRotated(playerRotation,0,1,0);

			if(totalModels>0)
			{
				currentanimation = models[playerModelIndex].currentanimation;
				currentframe = models[playerModelIndex].animations[currentanimation].currentframe;				
				drawModel(models[playerModelIndex].animations[currentanimation].frames[currentframe]);
			}
		glPopMatrix();
/*
		glBegin(GL_LINE_LOOP);
			for(int i=0;i<100;++i)
				glVertex3d(cos(i*2*PI/100.0)*playerCyl.radius,playerCyl.top,sin(i*2*PI/100.0)*playerCyl.radius);
		glEnd();

		glBegin(GL_LINE_LOOP);
			for(int i=0;i<100;++i)
				glVertex3d(cos(i*2*PI/100.0)*playerCyl.radius,playerCyl.bottom,sin(i*2*PI/100.0)*playerCyl.radius);
		glEnd();
*/
		break;
	default:
		break;
	}
	
	for(i=0;i<totalEntities;++i)
	{
		if(strcmp(entities[i].type,"Light")==0)
		{
			GLfloat light_position[] = {entities[i].point.X, entities[i].point.Y, entities[i].point.Z, 1.0};
			glLightfv(GL_LIGHT0+i, GL_POSITION, light_position);
		}
	}
	
	for(i=0; i<totalEnemies; ++i)
	{
		if(enemies[i].health>0 || enemies[i].dying==true)
		{
			glPushMatrix();
			glTranslated(enemies[i].position.X,enemies[i].position.Y,enemies[i].position.Z);
			glRotated(enemies[i].rotation,0,1,0);
			int currentanimation = enemies[i].currentanimation;
			int currentframe = models[enemies[i].modelIndex].animations[currentanimation].currentframe;
			drawModel(models[enemies[i].modelIndex].animations[currentanimation].frames[currentframe]);
			glPopMatrix();
		}
	}

	glPushMatrix();

	for(o=0; o<totalObjects; ++o)
	{
		if(objects[o].material==-1)
			setDefaultMaterial();
		else
			setmaterial(objects[o].material);
		

		for(i=objects[o].firstFace; i<objects[o].firstFace+objects[o].numFaces; ++i)	
		{			
//			facesdrawn++;
			glBegin(GL_POLYGON);
			for(k=0; k<faces[i].numPoints; ++k)
			{					
				glNormal3d(	normals[faces[i].normIndex[k]].X,
							normals[faces[i].normIndex[k]].Y,
							normals[faces[i].normIndex[k]].Z);
				glTexCoord2d(	texCoords[faces[i].texIndex[k]].X,
								texCoords[faces[i].texIndex[k]].Y);
				glVertex3d( vertices[faces[i].vertIndex[k]].X,
							vertices[faces[i].vertIndex[k]].Y,
							vertices[faces[i].vertIndex[k]].Z);
			}
			glEnd();
		}
	}
	glPopMatrix();

	showHealthMeter();
	showCrossHairs();

	if(showMenu)
		displayMenu(showMenuIndex);

	if(console)
		drawConsole();

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	if(h==0)
		h=1;

	actualX = w;
	actualY = h;
	mouseMult=360.0/actualX;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);

	gluPerspective(45, (GLdouble)w/(GLdouble)h, nClip, fClip);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void enterGameMode()
{
	glutGameModeString(screenSettings[screenResIndex]);
	if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) 
	{
		glutDestroyWindow(windowID);
		glutEnterGameMode();
		init();
	}			
}

void leaveGameMode()
{
	if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE) != 0)
	{
		glutLeaveGameMode();
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(windowWidth, windowHeight);
		glutCreateWindow("OpenFPS");
		windowID=glutGetWindow();
		init();
	}
}

void deInit()
{
	int i;

	for(i=0; i<totalMenus; ++i)
	{
		for(int j=0; j<menus[i].numMenuItems; ++j)
		{
			delete [] menus[i].menuItems[j].command;
			delete [] menus[i].menuItems[j].text;
		}
		delete [] menus[i].menuItems;
		delete [] menus[i].name;
		delete [] menus[i].text;
	}
	delete [] menus;

	for(i=0; i<NUM_SOUND_SOURCES+totalEnemies;++i)
		alSourceStop(SOUNDsource[i]);
}

void processConsoleCommand(char *command)
{
	char verb[80];
	char noun[80];
	bool foundVerbNoun = false;
	int i;

	toConsole(command);

	for(i=0; i<strlen(command); ++i)
	{
		if(command[i]==' ')
		{			
			strncpy(verb,command,i);
			foundVerbNoun = true;
			verb[i]='\0';

			for(unsigned int j=0; j<strlen(command)-i; ++j)
			{
				noun[j]=command[i+j+1];
			}
			noun[strlen(command)-i]='\0';
		}
	}

	if(foundVerbNoun==false)
		strcpy(verb,command);
	
	if(strcmp(verb,"load")==0)
	{
		toConsole("   loading level: ");
		toConsoleCat(noun);
		clearAll();
		loadAll(noun);
	}
	else if(strcmp(verb,"quit")==0)
	{
		toConsole("Quitting...");
		deInit();
		alutExit();
		exit(0);
	}
	else if(strcmp(verb,"clear")==0)
	{
		toConsole("Clearing Memory");
		clearAll();
		initializeAll();
	}
	else if(strcmp(verb,"clipping")==0)
	{
		if(strcmp(noun,"on")==0)
		{
			if(clipping)
				toConsole("Clipping is already on");
			else
			{
				toConsole("Clipping enabled");
				clipping=true;
			}
		}
		else if(strcmp(noun,"off")==0)
		{
			if(clipping)
			{
				toConsole("Clipping disabled");
				clipping=false;
			}
			else
				toConsole("Clipping is already off");
		}
		else
		{
			toConsole(noun);
			toConsoleCat(" is not a valid clipping state");
		}
	}
	else if(strcmp(verb,"grav")==0)
	{
		if(strcmp(noun,"on")==0)
		{
			if(physics)
				toConsole("Gravity is already on");
			else
			{
				toConsole("Gravity enabled");
				physics=true;
			}
		}
		else if(strcmp(noun,"off")==0)
		{
			if(physics)
			{
				toConsole("Gravity disabled");
				physics=false;
			}
			else
				toConsole("Gravity is already off");
		}
		else
		{
			toConsole(noun);
			toConsoleCat(" is not a valid gravity state");
		}
	}
	else if(strcmp(verb,"lighting")==0)
	{
		if(strcmp(noun,"on")==0)
		{
			if(lighting)
				toConsole("Lighting is already on");
			else
			{
				toConsole("Lighting enabled");
				lighting=true;
			}
		}
		else if(strcmp(noun,"off")==0)
		{
			if(lighting)
			{
				toConsole("Lighting disabled");
				lighting=false;
			}
			else
				toConsole("Lighting is already off");
		}
		else
		{
			toConsole(noun);
			toConsoleCat(" is not a valid lighting state");
		}
	}
	else if(strcmp(verb,"displaymenu")==0)
	{
		bool foundMenu = false;
		for(i=0; i<totalMenus; ++i)
		{
			if(strcmp(noun,menus[i].name)==0)
			{
				showMenu = true;
				showMenuIndex = i;
				foundMenu = true;
				glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
			}
		}
		if(!foundMenu)
		{
			toConsole("menu ");
			toConsoleCat(noun);
			toConsoleCat(" not found");			
		}
	}
	else if(strcmp(verb,"closemenu")==0)
	{
		showMenu = false;
		glutSetCursor(GLUT_CURSOR_NONE);
	}
	else if(strcmp(verb,"getkey")==0)
	{
		getNextKeypress = true;
		if(strcmp(noun,"walkup")==0)
			keytoset = &keys.walkUp;
		else if(strcmp(noun,"walkdown")==0)
			keytoset = &keys.walkDown;
		else if(strcmp(noun,"strafel")==0)
			keytoset = &keys.strafeL;
		else if(strcmp(noun,"strafer")==0)
			keytoset = &keys.strafeR;
		else if(strcmp(noun,"jump")==0)
			keytoset = &keys.jump;
		else if(strcmp(noun,"cameramode")==0)
			keytoset = &keys.cameraMode;
		else if(strcmp(noun,"console")==0)
			keytoset = &keys.console;
		else if(strcmp(noun,"menu")==0)
			keytoset = &keys.menu;
		else
			toConsole("Bad Key");
	}
	else if(strcmp(verb,"fullscreenSel")==0)
	{
		if(fullscreen==false)
		{
			fullscreen=true;
			enterGameMode();
		}
		else
		{
			fullscreen=false;
			leaveGameMode();
		}
	}
	else if(strcmp(verb,"resUp")==0)
	{
		if(screenResIndex<4)
			screenResIndex++;
	}
	else if(strcmp(verb,"resDown")==0)
	{
		if(screenResIndex>0)
			screenResIndex--;
	}
	else if(strcmp(verb,"applyRes")==0)
	{
		if(fullscreen==true)
			enterGameMode();
	}
	else if(strcmp(verb,"togglesound")==0)
	{
		if(Audio)
			Audio=false;
		else
			Audio=true;
	}
	else if(strcmp(verb,"die")==0)
	{
		if(!playerDead)
			Die();
	}
	else if(strcmp(verb,"live")==0)
	{
		if(playerDead)
			Live();
	}
	else
	{
		toConsole("could not parse: (");
		toConsoleCat(consoleCommand);
		toConsoleCat(") - ");
		if(foundVerbNoun)
		{
			toConsoleCat("(");
			toConsoleCat(verb);
			toConsoleCat(") is not a valid command");
		}
		else
			toConsoleCat("Syntax Error");
	}

	for(i=0;i<80;++i)
		consoleCommand[i]='\0';
	consoleCursorPosition=0;
	glutPostRedisplay();
}

void specialKey(int key, int x, int y)
{
	if(console==true)
	{
		switch(key)
		{
		case GLUT_KEY_LEFT:
			if (consoleCursorPosition>0)
				consoleCursorPosition--;
			break;
		case GLUT_KEY_RIGHT:
			if(consoleCommand[consoleCursorPosition]!='\0')
				consoleCursorPosition++;
				break;
		}
	}
			
}

void keyboard(unsigned char key, int x, int y)
{
	int i;
	if(getNextKeypress)
	{
		*keytoset = key;
		getNextKeypress=false;
	}
	else if (console==false)
	{
		if(key==keys.menu)
		{
			if(showMenu)
				processConsoleCommand("closemenu");
			else
				processConsoleCommand("displaymenu mainmenu");
		}
		else if (key==keys.walkUp && playerDead==false)
			moveUD=1;
		else if (key==keys.walkDown && playerDead==false)
			moveUD=-1;
		else if (key==keys.strafeL && playerDead==false)
			moveLR=-1;
		else if (key==keys.strafeR && playerDead==false)
			moveLR=1;
		else if (key==keys.jump && playerDead==false)
		{
			if(Audio)
			{
				alSourcePlay(SOUNDsource[1]);
			}
			playerJump = true;
			playerJumping=playerJumpThrust;
		}
		else if(key==keys.cameraMode)
		{
			if(viewMode==1)
				viewMode=2;
			else
				viewMode=1;
		}
		else if(key==keys.console)
		{
			if(console)
				console=false;
			else
				console=true;		
		}
	}
	else
	{
		switch (key)
		{
		case 8:
			for(i=consoleCursorPosition; i<80;++i)
				consoleCommand[i-1]=consoleCommand[i];
			if(consoleCursorPosition>0)
				consoleCursorPosition--;
			break;
		case 13:
			processConsoleCommand(consoleCommand);
			//console=false;
			break;
		case '`':
		case '~':
			console=false;
			break;
		default:
			consoleCommand[consoleCursorPosition++]=key;
			break;
		}
	}
	
	glutPostRedisplay();
}

void upKey(unsigned char key, int x, int y) 
{
	if(key==keys.jump)
	{
		if(Audio)
			alSourceStop(SOUNDsource[1]);
		playerJump=false;
	}
	else if(key==keys.walkUp || key==keys.walkDown)
		moveUD=0;
	else if(key==keys.strafeL || key==keys.strafeR)
		moveLR=0;
}

void idle()
{	
	glutPostRedisplay();
}

void processPhysics()
{
	if(playerJump)
	{
		playerJumping = playerJumpThrust;

		playerJumping-=gravity;
		playerPosition.Y+=playerJumping;
		playerView.Y+=playerJumping;
	}
	else
	{
		playerPosition.Y-=gravity;
		playerView.Y-=gravity;	
	}
}

void timerOne(int value)
{
	bool stand = true;
	ALfloat sourceP[3];
	ALfloat sourceV[3];
	int i;

	for(i=0; i<totalEnemies; ++i)
		models[enemies[i].modelIndex].currentanimation = enemies[i].currentanimation;

	if(moveUD!=0)
	{
		if(totalModels>0) models[playerModelIndex].currentanimation = 1;
		currentSpeed=playerSpeed*moveUD;
		movePlayer();
		stand=false;
	}
	if(moveLR!=0)
	{
		if(totalModels>0) models[playerModelIndex].currentanimation = 1;
		currentSpeed=playerSpeed*moveLR;
		strafePlayer();
		stand=false;
	}
	if(totalModels>0)
	{
		if(stand)
			models[playerModelIndex].currentanimation = 0;
		if(playerJump)
			models[playerModelIndex].currentanimation = 2;
	}
	
	if(physics)
		processPhysics();
	
	if(clipping)
		checkCollision(playerPosition, true, playerRadius);	

	moveEnemy();

	if(Audio)
	{
		SOUNDlistenerP[0]=playerPosition.X;
		SOUNDlistenerP[1]=playerPosition.Y;
		SOUNDlistenerP[2]=playerPosition.Z;

		alListenerfv(AL_POSITION,SOUNDlistenerP);
		alSourcefv(SOUNDsource[0],AL_POSITION,SOUNDlistenerP);
		alSourcefv(SOUNDsource[1],AL_POSITION,SOUNDlistenerP);
		alSourcefv(SOUNDsource[2],AL_POSITION,SOUNDlistenerP);
		alSourcefv(SOUNDsource[3],AL_POSITION,SOUNDlistenerP);

		for(i=NUM_SOUND_SOURCES;i<NUM_SOUND_SOURCES+totalEnemies;++i)
		{
			sourceP[0]=enemies[i-NUM_SOUND_SOURCES].position.X;
			sourceP[1]=enemies[i-NUM_SOUND_SOURCES].position.Y;
			sourceP[2]=enemies[i-NUM_SOUND_SOURCES].position.Z;

			sourceV[0]=0;
			sourceV[1]=0;
			sourceV[2]=0;

			alSourcefv(SOUNDsource[i],AL_POSITION,sourceP);
			alSourcefv(SOUNDsource[i],AL_VELOCITY,sourceV);
		}
	}

	if(playerRecoverTime>0)
		playerRecoverTime--;

	glutPostRedisplay();
	glutTimerFunc(5, timerOne, 0);
}

void timerTwo(int value)
{
	if(totalModels>0)
	{
		for(int i=0; i<totalEnemies; ++i)
		{
			if(models[enemies[i].modelIndex].animations[enemies[i].currentanimation].currentframe<
				models[enemies[i].modelIndex].animations[enemies[i].currentanimation].numFrames-1)
				models[enemies[i].modelIndex].animations[enemies[i].currentanimation].currentframe++;
			else
			{
				if(enemies[i].dying==false)
					models[enemies[i].modelIndex].animations[enemies[i].currentanimation].currentframe=0;
				else
					enemies[i].dying=false;
			}
		}

		if(models[playerModelIndex].animations[models[playerModelIndex].currentanimation].currentframe<
			models[playerModelIndex].animations[models[playerModelIndex].currentanimation].numFrames-1)
			models[playerModelIndex].animations[models[playerModelIndex].currentanimation].currentframe++;
		else
			models[playerModelIndex].animations[models[playerModelIndex].currentanimation].currentframe=0;
	}
	glutPostRedisplay();
	glutTimerFunc(80, timerTwo, 0);
}

void Shoot()
{
	alSourcePlay(SOUNDsource[0]);

	for(int i=0; i<totalEnemies; ++i)
	{
		pointStruct vector,vector2;
		GLdouble angle;
		GLdouble distance;

		distance = Distance(enemies[i].position,playerPosition);

		enemies[i].currentanimation = 1;
		vector.X = playerPosition.X - enemies[i].position.X;
		vector.Y = playerPosition.Y - enemies[i].position.Y;
		vector.Z = playerPosition.Z - enemies[i].position.Z;

		vector2.X = playerPosition.X - playerView.X;
		vector2.Y = playerPosition.Y - playerView.Y;
		vector2.Z = playerPosition.Z - playerView.Z;

		vector = Normalize(vector);
		vector2 = Normalize(vector2);

		angle = angleBetweenVectors(vector,vector2);

		if(angle*2 <= tan(models[enemies[i].modelIndex].radius/distance))
		{
			if(enemies[i].health-gunDamage>0)
				enemies[i].health-=gunDamage;
			else
			{
				enemies[i].health=0;
				enemyDie(i);
			}
			if(Audio)
			{
				alSourcePlay(SOUNDsource[3]);				
			}
		}	
	}
}

void doMouseStuff(int x, int y)
{
	if(!showMenu)
	{
		GLdouble mouseYDelta=(mouseY-y)*0.01;	
		mouseXDelta=(mouseX-x)*0.01;
		pointStruct vector;
		vector.X = playerView.X - playerPosition.X;
		vector.Y = playerView.Y - playerPosition.Y;
		vector.Z = playerView.Z - playerPosition.Z;

		pointStruct axis = getCross(vector, playerUp);
		axis = Normalize(axis);
		
		rotatePlayer(mouseYDelta, axis.X, axis.Y, axis.Z);
		rotatePlayer(mouseXDelta, 0, 1, 0);

		if(x!=actualX/2||y!=actualY/2)
			glutWarpPointer(actualX/2,actualY/2);
		x=actualX/2;
		y=actualY/2;
	}
	mouseX=x;
	mouseY=y;
}
void passive(int x, int y)
{
	doMouseStuff(x, y);
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	if(showMenu)
	{
		if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
		{
			if(!menuClick)
			{
				if(menuHover!=-1)
					processConsoleCommand(menus[showMenuIndex].menuItems[menuHover].command);
				menuClick=true;
			}
		}
		else
			menuClick=false;
	}
	else
	{
		if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
			Shoot();
	}

	doMouseStuff(x,y);
	glutPostRedisplay();
}


void getMenusInfo(char *menufile)
{
	char buffer[255];
	ifstream menu(menufile);
	int currentmenu;

	totalMenus=0;

	if (!menu.is_open())
		cout << "Could not open menu file, using console-only mode";
	else
	{
		do
		{
			menu.getline(buffer, 255, '"');
			menu.getline(buffer, 255, '"');
			if(strcmp(buffer,"name")==0)
				totalMenus++;
		} while (strcmp(buffer,"eof")!=0);
	
		cout << "MENUS:" << totalMenus << endl;
		menus = new menuStruct[totalMenus];
		currentmenu = 0;		
		menu.seekg(ios::beg);

		do
		{
			menu.getline(buffer, 255, '"');
			menu.getline(buffer, 255, '"');
			if(strcmp(buffer,"name")==0)
			{
				menus[currentmenu].fixedfont=false;
				menus[currentmenu].numMenuItems=0;
				currentmenu++;
			}
			if(strcmp(buffer,"menuitem")==0)
				menus[currentmenu-1].numMenuItems++;
		} while (strcmp(buffer,"eof")!=0);
	}
	for (int i=0; i<totalMenus; ++i)
	{
		cout << "setting menu " << i << " num " << menus[i].numMenuItems << endl;
		menus[i].menuItems = new menuItemStruct[menus[i].numMenuItems];
	}
}

void parseMenus(char *menufile)
{
	char buffer[255];
	ifstream menu(menufile);

	int currentmenu = 0;
	int currentmenuitem = 0;

	totalMenus=0;

	getMenusInfo(menufile);

	if (!menu.is_open())
		cout << "Could not open menu file, using console-only mode";
	else
	{
		do
		{
			menu.getline(buffer, 255, '"');
			menu.getline(buffer, 255, '"');
			if(strcmp(buffer,"name")==0)
			{
				menu.getline(buffer, 255, '"');
				menu.getline(buffer, 255, '"');
				menus[currentmenu].name = new char[strlen(buffer)+1];
				strcpy(menus[currentmenu].name,buffer);
				menu.getline(buffer, 255, '"');
				menu.getline(buffer, 255, '"');
				menus[currentmenu].text = new char[strlen(buffer)+1];
				strcpy(menus[currentmenu].text,buffer);
				currentmenuitem=0;
			}
			if(strcmp(buffer,"option")==0)
			{
				menu.getline(buffer, 255, '"');
				menu.getline(buffer, 255, '"');
				if(strcmp(buffer,"fixedfont")==0)
					menus[currentmenu].fixedfont = true;
			}
			if(strcmp(buffer,"menuitem")==0)
			{
				menu.getline(buffer, 255, '"');
				menu.getline(buffer, 255, '"');
				
				menus[currentmenu].menuItems[currentmenuitem].text = new char[strlen(buffer)+1];
				strcpy(menus[currentmenu].menuItems[currentmenuitem].text,buffer);
				menu.getline(buffer, 255, '"');
				menu.getline(buffer, 255, '"');
				menus[currentmenu].menuItems[currentmenuitem].command = new char[strlen(buffer)+1];
				strcpy(menus[currentmenu].menuItems[currentmenuitem].command,buffer);
				currentmenuitem++;
			}
			if(strcmp(buffer,"end")==0)
			{
				currentmenu++;
			}
		}
		while (strcmp(buffer,"eof")!=0);
	}
}

void setKeyDefaults()
{
	keys.walkUp = 'w';
	keys.walkDown = 's';
	keys.strafeL = 'a';
	keys.strafeR = 'd';
	keys.jump = ' ';
	keys.cameraMode = 'c';
	keys.console = '`';
	keys.menu = 27;
}

void init()
{
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);
	glutKeyboardUpFunc(upKey);
	glutIdleFunc(idle);
	glutPassiveMotionFunc(passive);
	glutMouseFunc(mouse);
	glutTimerFunc(5, timerOne, 0);
	glutTimerFunc(80, timerTwo, 0);

 	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);

}

void firstRun()
{
	glutWarpPointer(actualX/2,actualY/2);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutSetKeyRepeat( GLUT_KEY_REPEAT_OFF); 
	glutIgnoreKeyRepeat(1);

	for(int i=0; i<20; ++i)
		for(int j=0; j<80; ++j)
			consolebuffer[i][j]='\0';

	strcpy(screenSettings[0],"640x480:32");
	strcpy(screenSettings[1],"800x600:32");
	strcpy(screenSettings[2],"1024x768:32");
	strcpy(screenSettings[3],"1280x1024:32");
	strcpy(screenSettings[4],"1600x1200:32");
}

int main(int argc, char ** argv)
{
	alutInit(&argc, argv);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("OpenFPS");
	windowID=glutGetWindow();

	firstRun();
	init();

	setKeyDefaults();
	parseMenus("openfps.mnu");
	processConsoleCommand("displaymenu mainmenu");
	initializeAll();

	glutMainLoop();
	deInit();
	alutExit();
	return 0;
}
