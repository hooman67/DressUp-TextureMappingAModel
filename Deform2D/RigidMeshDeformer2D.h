#ifndef __RMSIMPLICIT_RIGID_MESH_DEFORMER_2D_H__
#define __RMSIMPLICIT_RIGID_MESH_DEFORMER_2D_H__

#include <map>
#include <set>
#include <limits>
#include "WmlLinearSystemExt.h"
#include "TriangleMesh.h"

namespace rmsmesh {

	struct Vertex {
		Eigen::Vector2f vPosition;
		Vertex(){ vPosition << 0, 0; }

		Vertex(float x, float y)
		{
			vPosition << x, y;
		}
	};

	struct Triangle {
		unsigned int nVerts[3];

		// definition of each vertex in triangle-local coordinate system
		Eigen::Vector2f vTriCoords[3];

		// un-scaled triangle
		Eigen::Vector2f vScaled[3];

		// pre-computed matrices for triangle scaling step
		Wml::GMatrixd mF, mC;
	};


class RigidMeshDeformer2D {
public:

	//hs
	std::vector<Vertex> hs_m_vInitialVertices;
	std::vector<Triangle> hs_m_vTriangles;

	const std::vector<Vertex> & getDeformedVerts() const
	{
		return m_vDeformedVerts;
	}
	const std::vector<Triangle> & getTriangles() const
	{
		return m_vTriangles;
	}
	Vertex hsGetVertex(unsigned int i);
	unsigned int hsfindClosestVertex(float x, float y);
	void clearAll(){
//		hs_m_vInitialVertices.clear();
//		hs_m_vTriangles.clear();
	//	m_vInitialVerts.clear();
	//	m_vDeformedVerts.clear();
	//	m_vTriangles.clear();
		m_vConstraints.clear();
	}





	RigidMeshDeformer2D( );
	~RigidMeshDeformer2D() {};


	void ForceValidation() { ValidateSetup(); }
	void RemoveHandle( unsigned int nHandle );



	//! nHandle is vertex ID
	void SetDeformedHandle( unsigned int nHandle, const Eigen::Vector2f & vHandle );


	void UnTransformPoint( Eigen::Vector2f & vTransform );

/*
 * mesh handling
 */
	void InitializeFromMesh( TriangleMesh * pMesh );
	void UpdateDeformedMesh( TriangleMesh * pMesh, bool bRigid );

	//hs
	void buildTriangleMeshFromDeformer(TriangleMesh * pMesh);
	void buildDeformerMesh();
	unsigned int FindHitVertex(float x, float y);
	void copyDefToInVert(){
		m_vInitialVerts.clear();
		for (int i = 0; i < m_vDeformedVerts.size(); i++){
			m_vInitialVerts.push_back(m_vDeformedVerts[i]);
		}
	}


/*
 * debug
 */
	const Eigen::Vector2f * GetTriangleVerts( unsigned int nTriangle ) { return m_vTriangles[nTriangle].vScaled; }

protected:
	std::vector<Vertex> m_vInitialVerts;
	std::vector<Vertex> m_vDeformedVerts;
	
	std::vector<Triangle> m_vTriangles;




	struct Constraint {
		unsigned int nVertex;
		Eigen::Vector2f vConstrainedPos;

		Constraint() { nVertex = 0; vConstrainedPos = Eigen::Vector2f::Zero(); }
		Constraint( unsigned int nVert, const Eigen::Vector2f & vPos ) { nVertex = nVert; vConstrainedPos = vPos; }

		bool operator<(const Constraint & c2) const
			{ return nVertex < c2.nVertex; }
	};

	std::set<Constraint> m_vConstraints;
	void UpdateConstraint( Constraint & cons );


	bool m_bSetupValid;
	void InvalidateSetup() { m_bSetupValid = false; }
	void ValidateSetup();


	Wml::GMatrixd m_mFirstMatrix;
	std::vector<unsigned int> m_vVertexMap;
	Wml::GMatrixd m_mHXPrime, m_mHYPrime;
	Wml::GMatrixd m_mDX, m_mDY;

	Wml::LinearSystemExtd::LUData m_mLUDecompX, m_mLUDecompY;

	void PrecomputeOrientationMatrix();
	void PrecomputeScalingMatrices( unsigned int nTriangle );
	void PrecomputeFittingMatrices();

	void ValidateDeformedMesh( bool bRigid );
	void UpdateScaledTriangle( unsigned int nTriangle );
	void ApplyFittingStep();

	Eigen::Vector2f GetInitialVert( unsigned int nVert ) 
	  { return Eigen::Vector2f( m_vInitialVerts[ nVert ].vPosition.x(), m_vInitialVerts[ nVert ].vPosition.y() ); }
};


} // namespace rmsimplicit


#endif // __RMSIMPLICIT_RIGID_MESH_DEFORMER_2D_H__
