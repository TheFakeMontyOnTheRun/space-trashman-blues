using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;
using System;

//[ExecuteInEditMode]
public class Exportable : MonoBehaviour
{
    public static Dictionary<string, Exportable> GeneralTable = new Dictionary<string, Exportable>();
    public enum GeometryType { None, Cube, LeftNear, LeftFar, NorthWall, WestWall, Corner, RampNorth, RampEast, RampSouth, RampWest};

    public float ceilingHeight;
    public float floorHeight;
    public float floorRepetitions;
    public float ceilingRepetitions;
    public Material mainMaterial;
    public Material ceilingMaterial;
    public Material ceilingRepetitionsMaterial;
    public Material floorMaterial;
    public Material floorRepetitionsMaterial;

    public bool blockVisibility;
    public bool blockMovement;
    public bool repeatMainTexture;
    public bool needsAlphaTest;
    public string representation;
    public GeometryType geometryType;

    static Shader specularShader;

    void Update()
    {
    }

    public void CopyFrom(Exportable other ) {
        ceilingHeight = other.ceilingHeight;
        floorHeight = other.floorHeight;
        floorRepetitions = other.floorRepetitions;
        ceilingRepetitions = other.ceilingRepetitions;
        mainMaterial = other.mainMaterial;
        ceilingMaterial = other.ceilingMaterial;
        ceilingRepetitionsMaterial = other.ceilingRepetitionsMaterial;
        floorMaterial = other.floorMaterial;
        floorRepetitionsMaterial = other.floorRepetitionsMaterial;
        blockVisibility = other.blockVisibility;
        blockMovement = other.blockMovement;
        needsAlphaTest = other.needsAlphaTest;
    }

    void cloneMaterialFor(Material originalMat, Renderer destination, float scaleX, float scaleY) {
        
        var newMat = new Material(specularShader);
        newMat.mainTexture = originalMat.mainTexture;
        newMat.mainTexture.filterMode = FilterMode.Point;
        newMat.mainTextureScale = new Vector2(scaleX, scaleY);
        destination.sharedMaterial = newMat;     
    }

    GameObject createObjectForGeometryType(GeometryType type) {
        
        switch (type) {
            case GeometryType.Cube:
                {
                    var GO = GameObject.CreatePrimitive(PrimitiveType.Cube);

                    if (repeatMainTexture)
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, (ceilingHeight - floorHeight));
                    }
                    else
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
                    }
                    return GO;
                }
            case GeometryType.LeftFar: {
                    var parentGO = new GameObject();
                    var GO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    GO.transform.parent = parentGO.transform;
                    GO.transform.localScale = new Vector3(0.1f, 1.0f, 0.1f * (float)Math.Sqrt(2.0));
                    GO.transform.rotation = Quaternion.Euler(0, 45.0f, 90.0f);

                    if (repeatMainTexture)
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), (ceilingHeight - floorHeight), 1.0f);
                    }
                    else
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
                    }

                    var otherGO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    otherGO.transform.parent = parentGO.transform;
                    otherGO.transform.localScale = new Vector3(0.1f, -1.0f, 0.1f * (float)Math.Sqrt(2.0));
                    otherGO.transform.rotation = Quaternion.Euler(0, 45.0f, 90.0f);
                    otherGO.GetComponent<MeshRenderer>().sharedMaterial = GO.GetComponent<MeshRenderer>().sharedMaterial;

                    return parentGO;
                }
            case GeometryType.LeftNear:
                {
                    var parentGO = new GameObject();
                    var GO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    GO.transform.parent = parentGO.transform;
                    GO.transform.localScale = new Vector3(0.1f, 1.0f, 0.1f * (float)Math.Sqrt(2.0));
                    GO.transform.rotation = Quaternion.Euler(0, 315.0f, 90.0f);

                    if (repeatMainTexture)
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), (ceilingHeight - floorHeight), 1.0f);
                    }
                    else
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
                    }

                    var otherGO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    otherGO.transform.parent = parentGO.transform;
                    otherGO.transform.localScale = new Vector3(0.1f, -1.0f, 0.1f * (float)Math.Sqrt(2.0));
                    otherGO.transform.rotation = Quaternion.Euler(0, 315.0f, 90.0f);
                    otherGO.GetComponent<MeshRenderer>().sharedMaterial = GO.GetComponent<MeshRenderer>().sharedMaterial;

                    return parentGO;
                }
            case GeometryType.NorthWall:
                {
                    var parentGO = new GameObject();
                    var GO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    GO.transform.parent = parentGO.transform;
                    GO.transform.localScale = new Vector3(0.1f, 1.0f, 0.1f);
                    GO.transform.position = new Vector3(0.5f, 0.0f, 0.0f);
                    GO.transform.rotation = Quaternion.Euler(0, 0.0f, 90.0f);

                    if (repeatMainTexture)
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), (ceilingHeight - floorHeight), 1.0f);
                    }
                    else
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
                    }

                    var otherGO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    otherGO.transform.parent = parentGO.transform;
                    otherGO.transform.localScale = new Vector3(0.1f, -1.0f, 0.1f);
                    otherGO.transform.position = new Vector3(0.5f, 0.0f, 0.0f);
                    otherGO.transform.rotation = Quaternion.Euler(0, 0.0f, 90.0f);
                    otherGO.GetComponent<MeshRenderer>().sharedMaterial = GO.GetComponent<MeshRenderer>().sharedMaterial;

                    return parentGO;
                }

            case GeometryType.WestWall:
                {
                    var parentGO = new GameObject();
                    var GO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    GO.transform.parent = parentGO.transform;
                    GO.transform.localScale = new Vector3(0.1f, 1.0f, 0.1f);
                    GO.transform.position = new Vector3(0f, 0.0f, 0.5f);
                    GO.transform.rotation = Quaternion.Euler(0, 90.0f, 90.0f);

                    if (repeatMainTexture)
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), (ceilingHeight - floorHeight), 1.0f);
                    }
                    else
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
                    }

                    var otherGO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    otherGO.transform.parent = parentGO.transform;
                    otherGO.transform.localScale = new Vector3(0.1f, -1.0f, 0.1f);
                    otherGO.transform.position = new Vector3(0.0f, 0.0f, 0.5f);
                    otherGO.transform.rotation = Quaternion.Euler(0, 90.0f, 90.0f);
                    otherGO.GetComponent<MeshRenderer>().sharedMaterial = GO.GetComponent<MeshRenderer>().sharedMaterial;

                    return parentGO;
                }

            case GeometryType.Corner:
                {
                    var parentGO = new GameObject();
                    var GO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    GO.transform.parent = parentGO.transform;
                    GO.transform.localScale = new Vector3(0.1f, 1.0f, 0.1f);
                    GO.transform.position = new Vector3(0.5f, 0.0f, 0.0f);
                    GO.transform.rotation = Quaternion.Euler(0, 0.0f, 90.0f);

                    if (repeatMainTexture)
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), (ceilingHeight - floorHeight), 1.0f);
                    }
                    else
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
                    }

                    var otherGO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    otherGO.transform.parent = parentGO.transform;
                    otherGO.transform.localScale = new Vector3(0.1f, -1.0f, 0.1f);
                    otherGO.transform.position = new Vector3(0.5f, 0.0f, 0.0f);
                    otherGO.transform.rotation = Quaternion.Euler(0, 0.0f, 90.0f);
                    otherGO.GetComponent<MeshRenderer>().sharedMaterial = GO.GetComponent<MeshRenderer>().sharedMaterial;

                    GO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    GO.transform.parent = parentGO.transform;
                    GO.transform.localScale = new Vector3(0.1f, 1.0f, 0.1f);
                    GO.transform.position = new Vector3(0f, 0.0f, 0.5f);
                    GO.transform.rotation = Quaternion.Euler(0, 90.0f, 90.0f);
                    GO.GetComponent<MeshRenderer>().sharedMaterial = otherGO.GetComponent<MeshRenderer>().sharedMaterial;

                    otherGO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    otherGO.transform.parent = parentGO.transform;
                    otherGO.transform.localScale = new Vector3(0.1f, -1.0f, 0.1f);
                    otherGO.transform.position = new Vector3(0.0f, 0.0f, 0.5f);
                    otherGO.transform.rotation = Quaternion.Euler(0, 90.0f, 90.0f);
                    otherGO.GetComponent<MeshRenderer>().sharedMaterial = GO.GetComponent<MeshRenderer>().sharedMaterial;

                    return parentGO;
                }
            case GeometryType.RampNorth:
            case GeometryType.RampEast:
            case GeometryType.RampSouth:
            case GeometryType.RampWest:
                {
                    float angleXZ = 0.0f;

                    switch (type) {
                        case GeometryType.RampNorth:
                            angleXZ = 0.0f;
                            break;
                        case GeometryType.RampEast:
                            angleXZ = 90.0f;
                            break;
                        case GeometryType.RampSouth:
                            angleXZ = 180.0f;
                            break;
                        case GeometryType.RampWest:
                            angleXZ = 270.0f;
                            break;
                    }

                    var parentGO = new GameObject();
                    var GO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    GO.transform.parent = parentGO.transform;
                    GO.transform.localScale = new Vector3(0.1f * (float)Math.Sqrt(2.0), 0.1f, 0.1f);
                    GO.transform.rotation = Quaternion.Euler(0, angleXZ, 45.0f);

                    if (repeatMainTexture)
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), (ceilingHeight - floorHeight), 1.0f);
                    }
                    else
                    {
                        cloneMaterialFor((mainMaterial as Material), GO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
                    }

                    var otherGO = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    otherGO.transform.parent = parentGO.transform;
                    otherGO.transform.localScale = new Vector3(0.1f * (float)Math.Sqrt(2.0), -0.1f, 0.1f);
                    otherGO.transform.rotation = Quaternion.Euler(0, angleXZ, 45.0f);
                    otherGO.GetComponent<MeshRenderer>().sharedMaterial = GO.GetComponent<MeshRenderer>().sharedMaterial;

                    return parentGO;
                }
            case GeometryType.None:
            default:
                return new GameObject();
        }
    }
 
    public void Apply() {

        
        var target = this;
        specularShader = Shader.Find("Unlit/Texture");

        if (floorMaterial != null ) {
            GameObject floorGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
            floorGO.transform.parent  = (target as Exportable).transform;
            floorGO.transform.localPosition = new Vector3(0, floorHeight, 0);
            floorGO.transform.localScale = new Vector3( 1, 0, 1 );
            floorGO.name = "floor";
            cloneMaterialFor((floorMaterial as Material), floorGO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);
        }


        if (ceilingMaterial != null) {
                GameObject ceilingGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingGO.transform.parent  = (target as Exportable).transform; 
                ceilingGO.transform.localPosition = new Vector3(0, ceilingHeight, 0);
                ceilingGO.transform.localScale = new Vector3( 1, 0, 1 );
                ceilingGO.name = "ceiling";
                cloneMaterialFor((ceilingMaterial as Material), ceilingGO.GetComponent<MeshRenderer>(), 1.0f, 1.0f);

        }

        if (mainMaterial != null && geometryType != GeometryType.None)
        {
            GameObject mainShapeGO = createObjectForGeometryType(geometryType);
            mainShapeGO.transform.parent = (target as Exportable).transform;
            mainShapeGO.transform.localPosition = new Vector3(0, (floorHeight +  ceilingHeight) / 2.0f, 0);
            mainShapeGO.transform.localScale = new Vector3(mainShapeGO.transform.localScale.x, (ceilingHeight - floorHeight), mainShapeGO.transform.localScale.z);
            mainShapeGO.name = "mainShape";
        }

        if (ceilingRepetitions > 0 && ceilingRepetitionsMaterial != null ) {
                GameObject ceilingRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                ceilingRepetitionsGO.transform.localPosition = new Vector3(0, ceilingHeight + (ceilingRepetitions / 2) + 0.001f, 0);
                ceilingRepetitionsGO.transform.localScale = new Vector3( 1, ceilingRepetitions, 1 );
                ceilingRepetitionsGO.name = "ceilingRepetition";
                cloneMaterialFor((ceilingRepetitionsMaterial as Material), ceilingRepetitionsGO.GetComponent<MeshRenderer>(), 1.0f, ceilingRepetitions);
        }

            if (floorRepetitions > 0 && floorRepetitionsMaterial != null) {
                GameObject floorRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                floorRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                floorRepetitionsGO.transform.localPosition = new Vector3(0, floorHeight - (floorRepetitions / 2) - 0.001f, 0);
                floorRepetitionsGO.transform.localScale = new Vector3( 1, floorRepetitions, 1 );
                floorRepetitionsGO.name = "floorRepetition";
                cloneMaterialFor((floorRepetitionsMaterial as Material), floorRepetitionsGO.GetComponent<MeshRenderer>(), 1.0f, floorRepetitions);
        }
        
    }
}
