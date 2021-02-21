package pt.b13h.wavefrontcompiler;

/**
 * 
 */

import br.odb.liboldfart.SimpleWavefrontOBJLoader;
import br.odb.liboldfart.WavefrontMaterial;
import br.odb.liboldfart.WavefrontMaterialLoader;
import br.odb.libstrip.GeneralTriangle;
import br.odb.libstrip.TriangleMesh;
import br.odb.libstrip.builders.GeneralTriangleFactory;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

/**
 * @author monty
 * 
 */
public class CompilerApp {

	public static void emit(float num) {
		if (num < 0 ) {
			System.out.print("-Mul( intToFix(" + ((int)(-num * 128)) + " ), bias ), /* " + num  + " */ ");
		} else {
			System.out.print("Mul( intToFix(" + ((int)(num * 128)) + " ), bias ), /* " + num  + " */ ");
		}
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) throws FileNotFoundException {
		GeneralTriangleFactory trigFactory = new GeneralTriangleFactory();
		WavefrontMaterialLoader materialLoader = new WavefrontMaterialLoader();
		SimpleWavefrontOBJLoader meshLoader = new SimpleWavefrontOBJLoader(trigFactory);
		List<WavefrontMaterial> materialList = materialLoader.parseMaterials(new FileInputStream("/Users/monty/Desktop/pile.mtl"));
		List<TriangleMesh> meshes = meshLoader.loadMeshes(new FileInputStream("/Users/monty/Desktop/pile.obj"), materialList);


		int total = 0;

		for (TriangleMesh m : meshes ) {
			total += m.faces.size();
		}

		System.out.println("uint8_t uvCoords[ " + total * 6 + "] = {");


		for (int c = 0; c < meshes.size(); ++c ) {
			(meshes.get(c).faces).sort(new Comparator() {

				float compoundZ(GeneralTriangle trig) {
					float totalZ = 0.0f;

					totalZ += trig.z0 + trig.z1 + trig.z2;

					return totalZ;
				}

				@Override
				public int compare(Object o1, Object o2) {
					return compoundZ((GeneralTriangle)o1) > compoundZ((GeneralTriangle)o2) ? 1: -1;
				}
			});
		}

		for (TriangleMesh m : meshes ) {

			for (GeneralTriangle trig : m.faces) {
				for (float f : trig.getTextureCoordinates()) {
					System.out.print( ((int)(f * 32)) + ", ");
				}
				System.out.println();
			}
		}
		System.out.println("};");

		System.out.println("FixP_t bias = Div(intToFix(1), intToFix(128));");

		System.out.println("FixP_t coords[ " + total * 9 + "] = {" );

		for (TriangleMesh m : meshes ) {

			for (GeneralTriangle trig : m.faces ) {
				emit(trig.x0 );
				emit(trig.y0 );
				emit(trig.z0 );
				System.out.println();
				emit(trig.x1 );
				emit(trig.y1 );
				emit(trig.z1 );
				System.out.println();
				emit(trig.x2 );
				emit(trig.y2 );
				emit(trig.z2 );
				System.out.println("\n");
			}
		}
		System.out.println("};");

		System.out.println("    struct Mesh mesh;\n" +
				"    \n" +
				"    mesh.triangleCount = " + total + ";\n" +
				"    mesh.uvCoords = &uvCoords[0];\n" +
				"    mesh.geometry = &coords[0];");
	}
}