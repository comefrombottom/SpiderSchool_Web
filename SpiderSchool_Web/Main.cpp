# include <Siv3D.hpp> // Siv3D v0.6.12
#include"Game.h"
#include"Title.h"
#include"Prologue.h"
#include"Ending.h"

struct TouchInfo {
	int32 id;
	Vec2 pos;
};

// タッチ情報を取得する関数
Array<TouchInfo> GetMyTouches() {
# if SIV3D_PLATFORM(WEB)
	Array<TouchInfo> result = Array<TouchInfo>(EM_ASM_INT({ return window.myTouches.length; }));

	EM_ASM({
		const touches = window.myTouches;

		for (let i = 0; i < touches.length; i++) {
			const touch = touches[i];
			const touchPtr = $0 + i * 24; // TouchInfo のサイズに応じて調整

			const adjusted = siv3dAdjustPoint(touch.pageX, touch.pageY);

			setValue(touchPtr, touch.identifier, 'i32');
			setValue(touchPtr + 8, adjusted.x, 'double');
			setValue(touchPtr + 16, adjusted.y, 'double');
		}
		}, result.data());

	for (auto& touch : result)
	{
		touch.pos = Scene::ClientToScene(touch.pos);
	}

	return result;
# else
	return Array<TouchInfo>();
# endif
}

class TouchesType
{
private:
	Array<TouchInfo> m_touches;
	Array<TouchInfo> m_preTouches;
public:

	void update()
	{
		m_preTouches = m_touches;
		m_touches = GetMyTouches();
	}

	const Array<TouchInfo>& getTouches() const
	{
		return m_touches;
	}

	const Array<TouchInfo>& getPreTouches() const
	{
		return m_preTouches;
	}

	operator bool() const
	{
		return bool(m_touches);
	}

	Optional<TouchInfo> getTouch(int32 id) const
	{
		for (const auto& touch : m_touches)
		{
			if (touch.id == id)
			{
				return touch;
			}
		}
		return none;
	}

	TouchInfo lastTouch() const
	{
		if (m_touches.isEmpty())
		{
			throw Error(U"lastTouch() called when no touches are available.");
		}
		else
		{
			return m_touches.back();
		}
	}

	template<class T>
	TouchesType intersects(T&& shape) const
	{
		TouchesType result;
		for (const auto& touch : m_touches)
		{
			if (shape.intersects(touch.pos))
			{
				result.m_touches.push_back(touch);
			}
		}
		return result;
	}
};

TouchesType Touches;

# if SIV3D_PLATFORM(WEB)
EM_JS(void, setupMultiTouchHandler, (), {
	// グローバル変数を定義
	window.myTouches = [];

// タッチイベントの処理を設定
const canvas = Module['canvas'];

function updateTouches(e) {
  window.myTouches = Array.from(e.touches);
  //e.preventDefault(); // 任意：スクロール防止など
}

canvas.addEventListener("touchstart", updateTouches, false);
canvas.addEventListener("touchmove", updateTouches, false);
canvas.addEventListener("touchend", updateTouches, false);
	});


# endif

void Main()
{



# if SIV3D_PLATFORM(WEB)
	setupMultiTouchHandler();

	EM_ASM({
		let keyDownEvent = null;
		let timeoutId = null;

		addEventListener("keydown", function(event) {
			if (!event.isTrusted) {
				return;
			}
			keyDownEvent = event;
		});

		addEventListener("keyup", function(event) {
			if (!event.isTrusted) {
				return;
			}
			const keyUpEvent = event;
			if (keyDownEvent.timeStamp == keyUpEvent.timeStamp) {
				clearTimeout(timeoutId);
				dispatchEvent(keyDownEvent);
				timeoutId = setTimeout(function() {
					dispatchEvent(keyUpEvent);
					timeoutId = null;
				}, 100);
			}
		});
	});
# endif


	Game game;
	//FontAsset::Register(U"24", 24);
	FontAsset::Register(U"forEnding", FontMethod::MSDF, 40, Resource(U"mplus-1p-black_spiderschool.ttf"));
	TextureAsset::Register(U"sister", Resource(U"sister.png"));
	TextureAsset::Register(U"brother", Resource(U"brother.png"));
	TextureAsset::Register(U"TrampolineSpider", Resource(U"TrampolineSpiderx3.png"));
	TextureAsset::Register(U"RideSpider", Resource(U"RideSpiderx3.png"));
	TextureAsset::Register(U"SwingSpider", Resource(U"SwingSpiderx3.png"));
	TextureAsset::Register(U"Tiles", Resource(U"Tiles.png"));
	TextureAsset::Register( U"smartPhone", Resource(U"smartphone.png"));
	TextureAsset::Register(U"brother_icon", Resource(U"brother_icon.png"));
	TextureAsset::Register(U"sister_icon", Resource(U"sister_icon.png"));
	TextureAsset::Register(U"responPos", Resource(U"responPos.png"));
	TextureAsset::Register(U"textureObjects", Resource(U"textureObjects.png"));
	TextureAsset::Register(U"swingHoldPlace", Resource(U"swingHoldPlace.png"));


	Window::SetStyle(WindowStyle::Sizable);

	constexpr double stepSec = (1.0 / 200.0);

	
	double accumulatorSec = 0.0;

	enum class EScene {
		title,
		prologue,
		game,
		pause,
		ending,

	};
	EScene escene=EScene::title;
	Title title;
	Prologue prologue;

	GameResultInfo resultInfo;

	Ending ending;

	MSRenderTexture gameRenderTexture(Size(800, 600));

	double timeAfterMousePressed = 100;

	while (System::Update())
	{
		ClearPrint();
		Touches.update();

		switch (escene)
		{
		case EScene::title: {
			title.update();
			title.draw();
			if (title.sceneEnd()) {
				escene = EScene::prologue;
			}
		}
			break;
		case EScene::prologue: {
			prologue.update();
			if (prologue.sceneEnd()) {
				escene = EScene::game;
			}

		}
			break;
		case EScene::game: {

			RectF leftButtonArea{ 0,Scene::Height() - 300,300,300 };
			RectF rightButtonArea{ 300,Scene::Height() - 300,300,300 };

			RectF jumpButtonArea{ Scene::Width() - 300,Scene::Height() - 300,300,300 };

			bool leftPressed = (MouseL.pressed() && leftButtonArea.intersects(Cursor::PosF())) or Touches.intersects(leftButtonArea);
			bool rightPressed = (MouseL.pressed() && rightButtonArea.intersects(Cursor::PosF())) or Touches.intersects(rightButtonArea);
			bool jumpPressed = (MouseL.pressed() && jumpButtonArea.intersects(Cursor::PosF())) or Touches.intersects(jumpButtonArea);

			{
				ScopedRenderTarget2D rt{ gameRenderTexture.clear(Palette::White)};

				game.setFirstUpdate(true);
				for (accumulatorSec += Scene::DeltaTime(); stepSec <= accumulatorSec; accumulatorSec -= stepSec)
				{
					game.update(rightPressed, leftPressed, jumpPressed, stepSec);
					game.setFirstUpdate(false);
				}

			

				game.draw();
			}
			Graphics2D::Flush();

			gameRenderTexture.resolve();

			double scale = (Vec2(Scene::Size()) / gameRenderTexture.size()).minComponent();
			gameRenderTexture.scaled(scale).draw(Arg::topCenter(Scene::Rect().topCenter().asPoint()));

			if (timeAfterMousePressed < 5) {
				leftButtonArea.drawFrame(2, leftPressed ? Palette::Red : Palette::White);
				rightButtonArea.drawFrame(2, rightPressed ? Palette::Red : Palette::White);
				jumpButtonArea.drawFrame(2, jumpPressed ? Palette::Red : Palette::White);
			}
			

			if (auto result = game.getResult()) {
				resultInfo = *result;
				escene = EScene::ending;
				ending.setResult(result->timer, result->deadCount);
			}

		}
			break;
		case EScene::pause:
			break;
		case EScene::ending: {
			ending.update();
			double scale = (Vec2(Scene::Size()) / gameRenderTexture.size()).minComponent();
			gameRenderTexture.scaled(scale).draw(Arg::topCenter(Scene::Rect().topCenter().asPoint()));
			ending.draw();
		}
			break;
		default:
			break;
		}


		if (MouseL.pressed()) {
			timeAfterMousePressed = 0;
		}
		else {
			timeAfterMousePressed += Scene::DeltaTime();
		}
		
	}
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要な場合があります。
//
