#ifndef Header_FusionEngine_Renderer
#define Header_FusionEngine_Renderer

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionEntity.h"


namespace FusionEngine
{

	typedef boost::intrusive_ptr<Camera> CameraPtr;

	//! Defines a camera rectangle
	/*!
	* \todo Interpolated rotation toward movement direction
	* \todo Interpolated smooth movement
	*/
	class Camera : public GarbageCollected<Camera>, noncopyable
	{
	public:
		Camera(asIScriptEngine *engine);
		Camera(asIScriptEngine *engine, float x, float y);
		Camera(asIScriptEngine *engine, EntityPtr follow);
		~Camera();

		void SetMass(float mass);

		void SetOrigin(CL_Origin origin);

		void SetPosition(float x, float y);

		void SetAngle(float angle);

		//void SetOrientation(const Quaternion &orientation);

		void SetZoom(float scale);

		void SetParallaxCamera(const CameraPtr &main_camera, float distance);

		void SetFollowEntity(const EntityPtr &follow);

		enum FollowMode
		{
			FixedPosition,
			FollowInstant,
			FollowSmooth,
			Physical
		};

		void SetFollowMode(FollowMode mode);

		enum RotateMode
		{
			FixedAngle,
			MatchEntity,
			SlerpToMovementDirection,
			Spline
		};

		void SetAutoRotate(RotateMode mode);

		void JoinToBody(b2Body *body);

		b2Body *CreateBody(b2World *world);
		b2Body *GetBody() const;

		void Update(float split);

		CL_Origin GetOrigin() const;

		const CL_Vec2f &GetPosition() const;

		float GetAngle() const;

		float GetZoom() const;

		void EnumReferences(asIScriptEngine *engine);
		void ReleaseAllReferences(asIScriptEngine *engine);

		static void Register(asIScriptEngine *engine);

	protected:
		void defineBody();
		void createBody(b2World *world);

		FollowMode m_Mode;
		RotateMode m_AutoRotate;

		CL_Origin m_Origin;
		CL_Vec2f m_Position;
		float m_Angle;
		float m_Scale;

		// Main camera for paralax effect (i.e. the
		//  camera that this one leads / follows)
		CameraPtr m_MainCamera;
		float m_ParallaxDistance;

		EntityPtr m_FollowEntity;

		b2BodyDef m_BodyDefinition;
		b2Body *m_Body;
		b2Joint *m_Joint;
	};

	//! A render area
	/*!
	* \see Camera | Renderer
	*/
	class Viewport : public RefCounted
	{
	public:
		Viewport();
		Viewport(CL_Rect area);
		Viewport(CL_Rect area, CameraPtr camera);

		//! Sets the position within the graphics context
		void SetPosition(int left, int top);
		//! Sets the size of the render area
		void SetSize(int width, int height);

		const CL_Rect &GetArea() const;
		CL_Point GetPosition() const;
		CL_Size GetSize() const;

		void SetCamera(const CameraPtr &camera);
		CameraPtr GetCamera() const;

		Vector2 ToScreenCoords(const Vector2 &entity_position);
		Vector2 ToEntityCoords(const Vector2 &screen_position);

		static void Register(asIScriptEngine *engine);

	protected:
		CL_Rect m_Area;
		CameraPtr m_Camera;
	};

	typedef boost::intrusive_ptr<Viewport> ViewportPtr;

	/*!
	 * \brief
	 * Renders Entities
	 *
	 * \see
	 * Viewport | EntityManager
	 */
	class Renderer
	{
	protected:
		//typedef std::set<std::string> BlockedTagSet;

		typedef std::set<EntityPtr> EntitySet;

	public:
		//! Constructor
		Renderer(const CL_GraphicContext &gc);
		//! Destructor
		virtual ~Renderer();

		enum ViewportArea
		{
			ViewFull,
			ViewVerticalHalf,
			ViewHorizontalHalf,
			ViewQuarter
		};
		ViewportPtr CreateViewport(ViewportArea area);

		int GetContextWidth() const;
		int GetContextHeight() const;

		void Add(const EntityPtr &entity);
		void Remove(const EntityPtr &entity);

		void Clear();

		void ShowTag(const std::string &tag);
		void HideTag(const std::string &tag);

		//void AddViewport(ViewportPtr viewport);

		void Update(float split);
		void Draw(ViewportPtr viewport);

	protected:
		bool updateTags(const EntityPtr &entity) const;

		void drawRenderables(EntityPtr &entity, const CL_Rectf &cull_outside);

		void drawNormally(const CL_Rectf &cull_outside);
		void updateDrawArray();

		struct ChangingTagCollection
		{
			void show(const std::string &tag)
			{
				m_HiddenTags.erase(tag);
				m_ShownTags.insert(tag);
			}

			void hide(const std::string &tag)
			{
				m_ShownTags.erase(tag);
				m_HiddenTags.insert(tag);
			}

			void clear()
			{
				m_ShownTags.clear();
				m_HiddenTags.clear();
			}

			bool somethingWasShown() const
			{
				return !m_ShownTags.empty();
			}

			bool checkShown(const std::string &tag) const
			{
				return m_ShownTags.find(tag) == m_ShownTags.end();
			}

			bool wasShown(const EntityPtr &entity) const
			{
				for (StringSet::const_iterator it = m_ShownTags.begin(), end = m_ShownTags.end(); it != end; ++it)
				{
					if (entity->CheckTag(*it))
						return true;
				}

				return false;
			}

			bool checkHidden(const std::string &tag) const
			{
				return m_HiddenTags.find(tag) == m_HiddenTags.end();
			}

			bool wasHidden(const EntityPtr &entity) const
			{
				for (StringSet::const_iterator it = m_HiddenTags.begin(), end = m_HiddenTags.end(); it != end; ++it)
				{
					if (entity->CheckTag(*it))
						return true;
				}

				return false;
			}

			const StringSet &shownTags() const
			{
				return m_ShownTags;
			}

			const StringSet &hiddenTags() const
			{
				return m_HiddenTags;
			}

			std::set<std::string> m_ShownTags;
			std::set<std::string> m_HiddenTags;
		} m_ChangedTags;

		//StringSet m_ShownTags;
		StringSet m_HiddenTags;

		EntitySet m_Entities;

		bool m_EntityAdded;

		EntityArray m_EntitiesToDraw;

		RenderableArray m_Renderables;

		//ViewportArray m_Viewports;

		CL_GraphicContext m_GC;

	};

}

#endif