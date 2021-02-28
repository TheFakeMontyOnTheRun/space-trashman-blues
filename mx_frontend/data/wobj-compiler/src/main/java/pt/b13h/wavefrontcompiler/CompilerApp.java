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

import java.io.*;
import java.nio.ByteBuffer;
import java.util.Comparator;
import java.util.List;

/**
 * @author monty
 * 
 */
public class CompilerApp {

	static int shift = (int) Math.pow(2, 16);

	public static void emitFP(ByteBuffer bb, float num) {

		int fixedPoint = 0;
		if (num < 0 ) {
			fixedPoint = -((int)(-num * shift));
		} else {
			fixedPoint = (int)(num * shift);
		}

		bb.put( (byte)(( fixedPoint & 0x000000FF) >> 0 ));
		bb.put( (byte)(( fixedPoint & 0x0000FF00) >> 8 ));
		bb.put( (byte)(( fixedPoint & 0x00FF0000) >> 16));
		bb.put( (byte)(( fixedPoint & 0xFF000000) >> 24));
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) throws FileNotFoundException {
		FileOutputStream fos = new FileOutputStream("../assets/output.mdl");
		ByteBuffer bb;
		GeneralTriangleFactory trigFactory = new GeneralTriangleFactory();
		WavefrontMaterialLoader materialLoader = new WavefrontMaterialLoader();
		SimpleWavefrontOBJLoader meshLoader = new SimpleWavefrontOBJLoader(trigFactory);
		List<WavefrontMaterial> materialList = materialLoader.parseMaterials(new FileInputStream("/Users/monty/Desktop/fighter.mtl"));
		List<TriangleMesh> meshes = meshLoader.loadMeshes(new FileInputStream("/Users/monty/Desktop/fighter.obj"), materialList);
		String textureName = meshes.get(0).faces.get(0).material.texture;
		textureName = textureName.substring(textureName.lastIndexOf("/") + 1).replace(".png", ".img");
		int total = 0;

		for (TriangleMesh m : meshes ) {
			total += m.faces.size();
		}

		bb = ByteBuffer.allocate(2 + (6 * total ) + (9 * 4 * total) + (textureName.length()) + 1);

		bb.put((byte )(total & 0xFF));
		bb.put((byte )((total >> 8) & 0xFF));


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
					bb.put( ((byte)(f * 32)));
				}
			}
		}

		for (TriangleMesh m : meshes ) {

			for (GeneralTriangle trig : m.faces ) {
				emitFP(bb, trig.x0 );
				emitFP(bb, trig.y0 );
				emitFP(bb, trig.z0 );

				emitFP(bb, trig.x1 );
				emitFP(bb, trig.y1 );
				emitFP(bb, trig.z1 );

				emitFP(bb, trig.x2 );
				emitFP(bb, trig.y2 );
				emitFP(bb, trig.z2 );
			}
		}
		bb.put((byte)textureName.length());
		bb.put(textureName.getBytes());


		try {
			fos.write(bb.array());
			fos.flush();
			fos.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}