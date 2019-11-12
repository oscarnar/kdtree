#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>
#include <vtkLineSource.h>
#include <vtkXYPlotActor.h>
#include <vtkChartXY.h>
#include <vtkPlaneSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkCamera.h>
using namespace std;

//Funcion ayuda para contruir los planos
vtkSmartPointer<vtkTransformPolyDataFilter> MakePlane(
  std::array<int, 2>& resolution, std::array<double, 3>& origin,
  std::array<double, 3>& point1, std::array<double, 3>& point2,
  std::array<double, 4>& wxyz, std::array<double, 3>& translate, double c[3])
{
  vtkSmartPointer<vtkPlaneSource> plane =
    vtkSmartPointer<vtkPlaneSource>::New();
  //plane->SetResolution(resolution[0], resolution[1]);
  //plane->SetOrigin(origin.data());
  plane->SetPoint1(point1.data());
  plane->SetPoint2(point2.data());
  plane->SetCenter(c);
  vtkSmartPointer<vtkTransform> trnf = vtkSmartPointer<vtkTransform>::New();
  trnf->RotateWXYZ(wxyz[0], wxyz[1], wxyz[2], wxyz[3]);
  //trnf->Translate(translate.data());
  vtkSmartPointer<vtkTransformPolyDataFilter> tpdPlane =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  tpdPlane->SetTransform(trnf);
  tpdPlane->SetInputConnection(plane->GetOutputPort());
  return tpdPlane;
}
bool sortbysec(const pair<int,int> &a,const pair<int,int> &b){
    return (a.second < b.second);
}
//Variable para los colores en VTK
vtkSmartPointer<vtkNamedColors> colors =
    vtkSmartPointer<vtkNamedColors>::New();

class Node{
public:
    vector<int> point;
    Node* left;
    Node* right;
    int axis;
public:
    Node(){
        this->left=0;
        this->right=0;
        this->axis=0;
        this->point.assign(3,0);
    }
    Node(vector<int> poi,int axi){
        this->left=0;
        this->right=0;
        this->point=poi;
        this->axis=axi;
    }
};
class Kdtree{
private:
    Node* root;
    int k;
public:
    Kdtree(int k1){
        this->root=NULL;
        this->k=k1;
    }
    //Funcion para ordenar los puntos de acuerdo al axis
    void ordenar(vector<vector<int>> &points, int axi){
        vector<pair<int,vector<int>>> temp;
        for(int i=0; i<points.size(); i++){
            int x = points[i][axi];
            temp.push_back(pair<int,vector<int>> (x,points[i]));
        }
        sort(temp.begin(),temp.end());
        points.clear();
        for(int i=0; i<temp.size(); i++){
            points.push_back(temp[i].second);
        }
    }
    //Constructor del kdtree
    Node* buil_kdtree(vector<vector<int>> points,int depth=0){
        int tam = points.size();
        int axis = depth % k;

        if(tam <= 0)
            return 0;
        if(tam == 1)
            return new Node(points[0],axis);
        int med = tam/2;
        vector<vector<int>> lef,rig;
        ordenar(points,axis);
        for(int i=0; i<tam; i++){
            if(i<med)
                lef.push_back(points[i]);
            if(i>med)
                rig.push_back(points[i]);
        }
        Node* temp = new Node(points[med],axis);
        temp->left = buil_kdtree(lef,depth + 1);
        temp->right = buil_kdtree(rig,depth + 1);

        return temp;
    }
    void buil(vector<vector<int>> points){
        this->root=buil_kdtree(points,0);
    }
    void print(Node* node){
        if(node == NULL)
            return;
        for(int i=0; i<node->point.size(); i++){
            cout<<node->point[i]<<",";
        }
        cout<<" ";
        print(node->left);
        print(node->right);
    }
    void print(){
        print(root);
    }
    //Funcion para graficar el arbol
    void drawTree(Node* node,ofstream &file){
        if(node == NULL)
            return;
        for(int i=0; i<node->point.size(); i++){
            file<<node->point[i];
            if(i < node->point.size()-1){
                file<<",";
            }
        }
        file<<endl;
        drawTree(node->left,file);
        drawTree(node->right,file);
    }
    void drawTree(){
        ofstream file("datos.txt");
        drawTree(root,file);
        file.close();
        system("python ../gra.py");
    }
    //Funcion para crear los puntos y los planos
    void draw(Node* no,vector<vtkSmartPointer<vtkActor>> &ac,
            vector<vtkSmartPointer<vtkActor>> &planes,
            vector<double> tam,
            vector<double> ori){
        if(no == NULL)
            return;
        vtkSmartPointer<vtkSphereSource> point =
            vtkSmartPointer<vtkSphereSource>::New();
        double x,y,z;
        x = no->point[0] * 1.0;
        y = no->point[1] * 1.0;
        z = no->point[2] * 1.0;
        point->SetCenter(x,y,z);
        point->SetRadius(4);

        vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(point->GetOutputPort());

        vtkSmartPointer<vtkActor> actor =
            vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        ac.push_back(actor);
        
        //Indicadores del origen y los contenedores
        vector<double> orileft=ori;
        vector<double> oriright=ori;
        vector<double> tamleft=tam;
        vector<double> tamright=tam;

        //draw planes
        vtkSmartPointer<vtkTransformPolyDataFilter> planesFil;
        array<int, 2> resolution{{10, 10}};
        array<double, 3> origin{{0,0,0}};
        
        array<double, 4> wxyz0{{0, 0, 0, 0}};
        array<double, 3> translate{{-0.5, -0.5, 0}};
        array<double, 4> wxyz1{{0, 0, 0, 0}};
        array<double, 4> wxyz2{{0, 0, 0, 0}};
        vtkSmartPointer<vtkActor> planeActor=vtkSmartPointer<vtkActor>::New();
        if(no->axis == 2){
            planeActor->GetProperty()->SetColor(colors->GetColor3d("Blue").GetData());
            array<double, 3> point1{{tam[0],0, 0}};
            array<double, 3> point2{{0,tam[1],0}};
            double c[3]={(tam[0]/2)+ori[0],(tam[1]/2)+ori[1],z};
            planesFil=MakePlane(resolution,origin,point1,point2,wxyz2,translate,c);
            oriright[2]=z;
            tamleft[2]=z-ori[2];
            tamright[2]=tam[2]-(z-ori[2]);
        }
        if(no->axis == 0){
            planeActor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
            array<double, 3> point1{{0, tam[1], 0}};
            array<double, 3> point2{{0,0, tam[2]}};
            double c[3]={x,(tam[1]/2)+ori[1],(tam[2]/2)+ori[2]};
            planesFil=MakePlane(resolution,origin,point1,point2,wxyz1,translate,c);
            oriright[0]=x;
            tamleft[0]=x-ori[0];
            tamright[0]=tam[0]-(x-ori[0]);
        }
        if(no->axis == 1){
            planeActor->GetProperty()->SetColor(colors->GetColor3d("Green").GetData());
            array<double, 3> point1{{tam[0], 0, 0}};
            array<double, 3> point2{{0,0,tam[2]}};
            double c[3]={(tam[0]/2)+ori[0],y,(tam[2]/2)+ori[2]};
            planesFil=MakePlane(resolution,origin,point1,point2,wxyz0,translate,c);
            oriright[1]=y;
            tamleft[1]=y-ori[1];
            tamright[1]=tam[1]-(y-ori[1]);
        }
        vtkSmartPointer<vtkPolyDataMapper> mappersPla = vtkSmartPointer<vtkPolyDataMapper>::New();
        mappersPla->SetInputConnection(planesFil->GetOutputPort());
        planeActor->SetMapper(mappersPla);
        planeActor->GetProperty()->SetOpacity(0.5);
        
        planes.push_back(planeActor);

        draw(no->left,ac,planes,tamleft,orileft);
        draw(no->right,ac,planes,tamright,oriright);
    }
    void scene(int tam){
        //Graficar el cubo, será el area
        vtkSmartPointer<vtkCubeSource> cube=vtkSmartPointer<vtkCubeSource>::New();
        cube->SetXLength(tam);
        cube->SetYLength(tam);
        cube->SetZLength(tam);
        vtkSmartPointer<vtkPolyDataMapper> cubeMapper=vtkSmartPointer<vtkPolyDataMapper>::New();
        cubeMapper->SetInputConnection(cube->GetOutputPort());
        vtkSmartPointer<vtkActor> cubeActor=vtkSmartPointer<vtkActor>::New();
        cubeActor->SetMapper(cubeMapper);
        cubeActor->SetPosition(tam/2,tam/2,tam/2);
        cubeActor->GetProperty()->SetOpacity(0.3);

        //Funcion para la escena
        vtkSmartPointer<vtkRenderer> renderer =
            vtkSmartPointer<vtkRenderer>::New() ;
        vtkSmartPointer<vtkRenderWindow> renderWin =
            vtkSmartPointer<vtkRenderWindow>::New();
        renderWin->SetWindowName("KdTree");
        renderWin->AddRenderer(renderer);
        vtkSmartPointer<vtkRenderWindowInteractor> renWinInt = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        renWinInt->SetRenderWindow(renderWin);
        vector<vtkSmartPointer<vtkActor>> points;
        vector<vtkSmartPointer<vtkActor>> lines;
        vector<vtkSmartPointer<vtkActor>> planes;
        vector<double> ori{0,0,0};
        vector<double> tama{tam*1.0,tam*1.0,1.0};
        draw(root,points,planes,tama,ori);
        for(int i=0; i<points.size(); i++){
            renderer->AddActor(points[i]);
            renderer->AddViewProp(planes[i]);
        }
        renderer->AddActor(cubeActor);
        renderWin->Render();
        renWinInt->Start();
    }
};
int main(int, char*[]){
    Kdtree tree(2);
    vector<vector<int>> vec;
    
    for(int i=0; i<50; i++){
        vec.push_back(vector<int> {rand()%298,rand()%295,0});
    }
    tree.buil(vec);
    tree.drawTree();
    tree.scene(300);
    int x,y,z;
    while(true){
        cin>>x>>y>>z;
        vec.push_back(vector<int>{x,y,z});
        tree.buil(vec);
        tree.scene(300);
    }
}
