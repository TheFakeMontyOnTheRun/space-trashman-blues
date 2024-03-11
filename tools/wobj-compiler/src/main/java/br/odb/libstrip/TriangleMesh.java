package br.odb.libstrip;

import br.odb.gameutils.math.Vec3;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * 
 * @author Daniel "Monty" Monteiro
 * 
 */
public class TriangleMesh implements Serializable {
	/**
	 * 
	 */
	private static final long serialVersionUID = 3375701151469728552L;
	
	final public List<GeneralTriangle> faces = new ArrayList<>();
	
	final public String name;
	private float[] cachedVertexData;
	private float[] cachedColorData;

	public void clear() {
		faces.clear();
		cachedColorData = null;
		cachedVertexData = null;
	}

	/**
	 * 
	 * @param mesh
	 */
	public TriangleMesh(String name, TriangleMesh mesh) {

		this(name);

		for (GeneralTriangle face : mesh.faces) {
			faces.add(face.makeCopy());
		}
	}

	public TriangleMesh(String name) {
		this.name = name;
	}

	/**
	 * 
	 */
	@Override
	public String toString() {
		
		StringBuilder sb = new StringBuilder();
		
		sb.append("<name>").append(name).append("</name>\n");

		for (GeneralTriangle isf : faces) {
			sb.append("\n").append(isf);
		}
		return sb.toString();
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + faces.hashCode();
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		return result;
	}



	public Vec3 getCenter() {
		float points = 0;
		Vec3 center = new Vec3();

		for ( GeneralTriangle t : faces ) {
			
			center.addTo( t.x0, t.y0, t.z0 );
			center.addTo( t.x1, t.y1, t.z1 );
			center.addTo( t.x2, t.y2, t.z2 );			
			points += 3.0f;
		}

		center.scale( 1.0f / points );

		return center;
	}

	
	/* (non-Javadoc)
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if ( !( obj instanceof TriangleMesh ) ) {
			return false;
		}
		TriangleMesh other = (TriangleMesh) obj;
		if (!faces.equals(other.faces)) {
			return false;
		}
		if (name == null) {
			if (other.name != null) {
				return false;
			}
		} else if (!name.equals(other.name)) {
			return false;
		}
		return true;
	}

	public void rotateXZ( float angleDegrees ) {
		
		float rad = (float) ( Math.PI / 180.0f );
		float sin = (float) Math.sin( angleDegrees * rad );
		float cos = (float) Math.sin( angleDegrees * rad );

		float x;
		float z;
		
		for ( GeneralTriangle trig : faces ) {
			
			x = trig.x0 * cos - trig.z0 * sin;
			z = trig.x0 * sin + trig.z0 * cos;
			trig.x0 = x;
			trig.z0 = z;
			
			x = trig.x1 * cos - trig.z1 * sin;
			z = trig.x1 * sin + trig.z1 * cos;
			trig.x1 = x;
			trig.z1 = z;			

			x = trig.x2 * cos - trig.z2 * sin;
			z = trig.x2 * sin + trig.z2 * cos;
			trig.x2 = x;
			trig.z2 = z;		
		}
		
		updateVertexData();
	}
	
	public void translateTo(Vec3 translation) {
		Vec3 center = this.getCenter();
		translate( translation.sub( center ) );
	}
	
	public void scale(float scaleFactor) {
		scale( new Vec3( scaleFactor, scaleFactor, scaleFactor ) );
	}
	
	
	public void scale(Vec3 scaleFactors) {

		for ( GeneralTriangle trig : faces ) {
			trig.x0 *= scaleFactors.x;
			trig.x1 *= scaleFactors.x;
			trig.x2 *= scaleFactors.x;
			trig.y0 *= scaleFactors.y;
			trig.y1 *= scaleFactors.y;
			trig.y2 *= scaleFactors.y;
			trig.z0 *= scaleFactors.z;
			trig.z1 *= scaleFactors.z;
			trig.z2 *= scaleFactors.z;
		}
		
		if (cachedVertexData != null) {

			for (int c = 0; c < cachedVertexData.length; c += 9) {

				cachedVertexData[c    ] *= scaleFactors.x;
				cachedVertexData[c + 1] *= scaleFactors.y;
				cachedVertexData[c + 2] *= scaleFactors.z;

				cachedVertexData[c + 3] *= scaleFactors.x;
				cachedVertexData[c + 4] *= scaleFactors.y;
				cachedVertexData[c + 5] *= scaleFactors.z;

				cachedVertexData[c + 6] *= scaleFactors.x;
				cachedVertexData[c + 7] *= scaleFactors.y;
				cachedVertexData[c + 8] *= scaleFactors.z;
			}
		}		
	}
	
	public void translate(Vec3 translation) {

		for ( GeneralTriangle trig : faces ) {
			trig.x0 += translation.x;
			trig.x1 += translation.x;
			trig.x2 += translation.x;
			trig.y0 += translation.y;
			trig.y1 += translation.y;
			trig.y2 += translation.y;
			trig.z0 += translation.z;
			trig.z1 += translation.z;
			trig.z2 += translation.z;
		}
		
		if (cachedVertexData != null) {

			for (int c = 0; c < cachedVertexData.length; c += 9) {

				cachedVertexData[c    ] += translation.x;
				cachedVertexData[c + 1] += translation.y;
				cachedVertexData[c + 2] += translation.z;

				cachedVertexData[c + 3] += translation.x;
				cachedVertexData[c + 4] += translation.y;
				cachedVertexData[c + 5] += translation.z;

				cachedVertexData[c + 6] += translation.x;
				cachedVertexData[c + 7] += translation.y;
				cachedVertexData[c + 8] += translation.z;
			}
		}		
	}

	
	void updateVertexData() {
		GeneralTriangle t;
		cachedVertexData = new float[9 * this.faces.size()];

		for (int c = 0; c < cachedVertexData.length; c += 9) {

			t = faces.get(c / 9);
			cachedVertexData[c] = t.x0;
			cachedVertexData[c + 1] = t.y0;
			cachedVertexData[c + 2] = t.z0;

			cachedVertexData[c + 3] = t.x1;
			cachedVertexData[c + 4] = t.y1;
			cachedVertexData[c + 5] = t.z1;

			cachedVertexData[c + 6] = t.x2;
			cachedVertexData[c + 7] = t.y2;
			cachedVertexData[c + 8] = t.z2;
		}
	}
	
	public float[] getVertexData() {

		if (cachedVertexData == null) {
			updateVertexData();
		}

		return cachedVertexData;
	}

	public float[] getColorData() {

		if (cachedColorData == null) {
			
			float[] colourData;
			GeneralTriangle t;
			cachedColorData = new float[4 * this.faces.size()];

			for (int c = 0; c < faces.size(); ++c) {

				t = faces.get(c);
				colourData = t.material.mainColor.getFloatData();
				cachedColorData[(c * 4)] = colourData[ 0 ];
				cachedColorData[(c * 4) + 1] = colourData[ 1 ];
				cachedColorData[(c * 4) + 2] = colourData[ 2 ];
				cachedColorData[(c * 4) + 3] = colourData[ 3 ];
			}
		}

		return cachedColorData;
	}
}
