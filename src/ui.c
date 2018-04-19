struct WidgetStyle
{
	u32 color;
};

enum WidgetKinds
{
	Widget_Command,
};
struct Widget
{
	i16 kind, style;
	i32 flags;
	f32 x, y;
	f32 w, h;

	union {
		struct {
			char chars[15];
			char nullTerm;
		};
		
		struct {
			char* text;
			isize textLength;
		};
	};

	i32 links[4];
	i32* output;
};

struct Gui
{
	struct Widget* current;
	struct Widget* widgets;
	struct WidgetStyles* styles;
	i32 count, capacity;
};


