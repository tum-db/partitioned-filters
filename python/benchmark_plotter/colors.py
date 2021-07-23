def blend(color, factor):
    return int(255 - (255 - color) * factor)


def rgb(red, green, blue, factor=1):
    """Return color as #rrggbb for the given color values."""
    return '#%02x%02x%02x' % (blend(red, factor), blend(green, factor), blend(blue, factor))


colors = {
    'blue': rgb(55, 126, 184),
    'lightblue': rgb(55, 126, 184, 0.6),
    'green': rgb(77, 175, 74),
    'lightgreen': rgb(77, 175, 74, 0.6),
    'orange': rgb(255, 127, 0),
    'lightorange': rgb(255, 127, 0, 0.75),
    'red': rgb(228, 26, 28),
    'lightred': rgb(228, 26, 28, 0.75),
    'black': rgb(0, 0, 0)
}
