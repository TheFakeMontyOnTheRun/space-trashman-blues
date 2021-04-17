using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//[ExecuteInEditMode]
public class Exportable : MonoBehaviour
{
    public static Dictionary<string, Exportable> GeneralTable = new Dictionary<string, Exportable>();

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

    

    void cloneMaterialFor(Material originalMat, Renderer destination, float scale) {
        
        var newMat = new Material(specularShader);
        newMat.mainTexture = originalMat.mainTexture;
        newMat.mainTextureScale = new Vector2(1.0f, scale);
        destination.sharedMaterial = newMat;     
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
            cloneMaterialFor((floorMaterial as Material), floorGO.GetComponent<MeshRenderer>(), 1.0f);
        }


        if (ceilingMaterial != null) {
                GameObject ceilingGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingGO.transform.parent  = (target as Exportable).transform; 
                ceilingGO.transform.localPosition = new Vector3(0, ceilingHeight, 0);
                ceilingGO.transform.localScale = new Vector3( 1, 0, 1 );
                ceilingGO.name = "ceiling";
                cloneMaterialFor((ceilingMaterial as Material), ceilingGO.GetComponent<MeshRenderer>(), 1.0f);

        }

        if (ceilingRepetitions > 0 && ceilingRepetitionsMaterial != null ) {
                GameObject ceilingRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                ceilingRepetitionsGO.transform.localPosition = new Vector3(0, ceilingHeight + (ceilingRepetitions / 2) + 0.001f, 0);
                ceilingRepetitionsGO.transform.localScale = new Vector3( 1, ceilingRepetitions, 1 );
                ceilingRepetitionsGO.name = "ceilingRepetition";
                cloneMaterialFor((ceilingRepetitionsMaterial as Material), ceilingRepetitionsGO.GetComponent<MeshRenderer>(), ceilingRepetitions);
        }

            if (floorRepetitions > 0 && floorRepetitionsMaterial != null) {
                GameObject floorRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                floorRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                floorRepetitionsGO.transform.localPosition = new Vector3(0, floorHeight - (floorRepetitions / 2) - 0.001f, 0);
                floorRepetitionsGO.transform.localScale = new Vector3( 1, floorRepetitions, 1 );
                floorRepetitionsGO.name = "floorRepetition";
                cloneMaterialFor((floorRepetitionsMaterial as Material), floorRepetitionsGO.GetComponent<MeshRenderer>(), floorRepetitions);
        }
        
    }
}
