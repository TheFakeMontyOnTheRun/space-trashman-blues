using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;
using System;

public class Exports : MonoBehaviour
{
    class PetEntry {

        public PetEntry(int floor, int ceiling, int geometryType, bool blockMovement, int textureIndex) {
            this.floor = floor;
            this.ceiling = ceiling;
            this.geometryType = geometryType;
            this.blockMovement = blockMovement;
            this.textureIndex = textureIndex;
        }

        public int floor;
        public int ceiling;
        public int geometryType;
        public bool blockMovement;
        public int textureIndex;
    };


    [MenuItem("Monty/Reset scene")]
    static void ResetLevel()
    {
        var objects = FindObjectsOfType(typeof(GameObject));

        foreach (GameObject child  in objects ) {
            DestroyImmediate(child.gameObject);
        }

        GameObject geometryRoot = new GameObject("Geometry");
        GameObject spawner;
        Material matRef1 = AssetDatabase.LoadAssetAtPath("Assets/Materials/tex1.mat", typeof(Material)) as Material;
        Material matRef2 = AssetDatabase.LoadAssetAtPath("Assets/Materials/tex2.mat", typeof(Material)) as Material;
    
        for ( int y = 0; y < 64; ++y ) {
            for (int x = 0; x < 64; ++x ) {
                spawner = new GameObject("tile" + x + "_" + y );
                spawner.transform.parent = geometryRoot.transform;
                spawner.transform.position = new Vector3(x, 0, y);
                

                spawner.AddComponent<Exportable>();
                spawner.GetComponent<Exportable>().floorMaterial = matRef1;
                spawner.GetComponent<Exportable>().ceilingMaterial = matRef1;
                spawner.GetComponent<Exportable>().ceilingHeight = 1.0f;
                spawner.GetComponent<Exportable>().representation = "1";
                spawner.GetComponent<Exportable>().Apply();
            }
        }

        GameObject playerSpawner = new GameObject("PlayerSpawner");
        playerSpawner.AddComponent<PlayerSpawner>();
        playerSpawner.transform.position = new Vector3( 1, 1  ,1 );
    }

    [MenuItem("Monty/Export Level as Data File")]
    static void ExportLevelFile()
    {
        Debug.Log("Doing stuff");
    }

    [MenuItem("Monty/Export Level as C Source File")]
    static void ExportLevelAsSource() {
        Debug.Log("Doing stuff in C");

        GameObject[] objects = GameObject.FindObjectsOfType(typeof(GameObject)) as GameObject[];

        string[,] map = new string[64, 64];
        PetEntry[] pet = new PetEntry[256];

        for (int y = 0; y < 64; ++y) {
            for ( int x = 0; x < 64; ++x ) {
                map[y, x] = "0";
            }
        }

        foreach (GameObject go in objects) {
            if (go.activeInHierarchy) {
                var exportbl = go.GetComponent(typeof(Exportable)) as Exportable;
                if (exportbl != null) {
                    int x = (int) (go.transform.position.x );
                    int z = 64 - (int) (go.transform.position.z );
                    print(go + ": " + go.name + " is active in hierarchy at ( " + x + ", " + z + " ) and is " + exportbl.representation);
                    
                    map[z, x] = exportbl.representation;
                    pet[Int32.Parse(map[z, x])] = new PetEntry(0, (int)go.transform.localScale.x, 0, false, 0);
                }
            }
        }


        for (int y = 1; y < 63; ++y)
        {
            for (int x = 1; x < 63; ++x)
            {
                if (map[y, x] == "0" && (
                    (Int32.Parse(map[y - 1, x]) > 1) ||
                    (Int32.Parse(map[y + 1, x]) > 1) ||
                    (Int32.Parse(map[y, x + 1]) > 1) ||
                    (Int32.Parse(map[y, x - 1]) > 1) ||
                    (Int32.Parse(map[y - 1, x - 1]) > 1) ||
                    (Int32.Parse(map[y - 1, x + 1]) > 1) ||
                    (Int32.Parse(map[y + 1, x - 1]) > 1) ||
                    (Int32.Parse(map[y + 1, x + 1]) > 1))) {

                    map[y, x] = "1";
                }
            }
        }

        string finalMap = "{\n";

        for (int y = 0; y < 64; ++y)
        {
            finalMap += "{ ";
            for (int x = 0; x < 64; ++x)
            {
                finalMap += map[y, x] + ", ";
            }
            finalMap += "},\n";
        }
        finalMap += "};\n";

        string petCode = "patterns[16]={\n";

        for (int c = 0; c < 256; ++c ) {

            if (pet[c] == null) {
                continue;
            }

            petCode += "{";
            petCode += pet[c].floor;
            petCode += ", ";
            petCode += pet[c].ceiling;
            petCode += ", ";
            petCode += pet[c].blockMovement;
            petCode += ", ";
            petCode += pet[c].geometryType;
            petCode += ", ";
            petCode += pet[c].textureIndex;
            petCode += " },\n";
        }

        petCode += "};";

        string path = "Assets/monty.txt";
        StreamWriter writer = new StreamWriter(path, false);
        writer.WriteLine(finalMap);
        writer.Close();

        AssetDatabase.ImportAsset(path);


        path = "Assets/pet.txt";
        writer = new StreamWriter(path, false);
        writer.WriteLine(petCode);
        writer.Close();

        AssetDatabase.ImportAsset(path);
    }
}
