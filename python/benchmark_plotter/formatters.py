from matplotlib.ticker import FuncFormatter


def kilos(x, pos):
    """The two args are the value and tick position"""
    return '%1.0f\\,K' % (x * 1e-3)


def millions(x, pos):
    """The two args are the value and tick position"""
    if x:
        return '%1.0f\\,M' % (x * 1e-6)
    else:
        return '0'


def billions(x, pos):
    """The two args are the value and tick position"""
    if x == 0:
        return '0'
    elif x < 1e8:
        return '%1.0f\\,M' % (x * 1e-6)
    elif x < 1e10:
        return '%1.1f\\,G' % (x * 1e-9)
    else:
        return '%1.0f\\,G' % (x * 1e-9)


def billions2(x, pos):
    """The two args are the value and tick position"""
    if x == 0:
        return '0'
    else:
        return '%1.0f\\,G' % (x * 1e-9)


def perc(x, pos):
    return '%.0f' % (x * 100) + '\%'


def perc2(x, pos):
    return '%.0f' % (x) + '\%'


kilos = FuncFormatter(kilos)
mills = FuncFormatter(millions)
gigs = FuncFormatter(billions)
gigs2 = FuncFormatter(billions2)
percent = FuncFormatter(perc)
percent2 = FuncFormatter(perc2)
