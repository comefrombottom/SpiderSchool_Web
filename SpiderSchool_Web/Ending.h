#pragma once
class Ending {
	double timer = 0;
	double clearTime = 0;
	double deadCount = 0;


	static constexpr RectF retryButtonRect = RectF{ Arg::topRight(800-20, 20), 150, 50 };
	bool retryButtonHover = false;
	bool retryButtonDown = false;

public:
	void setResult(double clearTime_, int32 deadCount_) {
		clearTime = clearTime_;
		deadCount = deadCount_;
	}
	void update(double delta = Scene::DeltaTime()) {
		timer += delta;
	}
	void draw() {
		Scene::Rect().draw(ColorF(1, timer * 0.5));

		double scale = (Vec2(Scene::Size()) / Vec2{800,600}).minComponent();
		Vec2 tl = (Vec2{ Scene::Width(),Scene::Height() } - Vec2{ 800,600 } * scale) / 2;
		Transformer2D tf{ Mat3x2::Scale(scale).translated(tl),TransformCursor::Yes };

		if (timer > 2) {
			FontAsset(U"forEnding")(U"ThankYouForPlaying!").drawAt(60,Vec2{400,300} + Vec2{0, -140}, Palette::Black);
			FontAsset(U"forEnding")(U"ClearTime:",clearTime,U"s").drawAt(50, Vec2{400,300 + 30}, Palette::Black);
			FontAsset(U"forEnding")(U"DeathCount:", deadCount).drawAt(40,Vec2{400,300} + Vec2{0,200}, Palette::Black);

			retryButtonHover = retryButtonRect.contains(Cursor::PosF());
			retryButtonRect.rounded(5).draw(retryButtonHover ? ColorF(0.8, 0.8, 0.8) : ColorF(1)).drawFrame(2, Palette::Black);
			FontAsset(U"forEnding")(U"Retry").drawAt(30, retryButtonRect.center(), Palette::Black);
			if (retryButtonHover && MouseL.down()) {
				retryButtonDown = true;
			}
		}
	}

	bool sceneEnd() {
		return retryButtonDown;
	}
};
