using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//[ExecuteInEditMode]
public class Exportable : MonoBehaviour
{
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
    public bool needsAlphaTest;
    public string representation;


    void Update()
    {
    }

    public void Apply() {
        var target = this;
            if (floorMaterial != null ) {
                GameObject floorGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                floorGO.transform.parent  = (target as Exportable).transform;
                floorGO.transform.localPosition = new Vector3(0, floorHeight, 0);
                floorGO.transform.localScale = new Vector3( 1, 0, 1 );
                floorGO.name = "floor";
                floorGO.GetComponent<MeshRenderer>().material = floorMaterial as Material;
            }


            if (ceilingMaterial != null) {
                GameObject ceilingGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingGO.transform.parent  = (target as Exportable).transform; 
                ceilingGO.transform.localPosition = new Vector3(0, ceilingHeight, 0);
                ceilingGO.transform.localScale = new Vector3( 1, 0, 1 );
                ceilingGO.name = "ceiling";
                ceilingGO.GetComponent<MeshRenderer>().material = ceilingMaterial as Material;
            }

            if (ceilingRepetitions > 0 && ceilingRepetitionsMaterial != null ) {
                GameObject ceilingRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                ceilingRepetitionsGO.transform.localPosition = new Vector3(0, ceilingHeight + (ceilingRepetitions / 2), 0);
                ceilingRepetitionsGO.transform.localScale = new Vector3( 1, ceilingRepetitions, 1 );
                ceilingRepetitionsGO.name = "ceilingRepetition";
                ceilingRepetitionsGO.GetComponent<MeshRenderer>().material = ceilingRepetitionsMaterial as Material;
            }

            if (floorRepetitions > 0 && floorRepetitionsMaterial != null) {
                GameObject floorRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                floorRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                floorRepetitionsGO.transform.localPosition = new Vector3(0, floorHeight - (floorRepetitions / 2), 0);
                floorRepetitionsGO.transform.localScale = new Vector3( 1, floorRepetitions, 1 );
                floorRepetitionsGO.name = "floorRepetition";
                floorRepetitionsGO.GetComponent<MeshRenderer>().material = floorRepetitionsMaterial as Material;
            }
    }
}
