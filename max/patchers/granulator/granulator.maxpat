{
    "patcher": {
        "fileversion": 1,
        "appversion": {
            "major": 9,
            "minor": 2,
            "revision": 0,
            "architecture": "x64",
            "modernui": 1
        },
        "classnamespace": "box",
        "rect": [ -295.0, -988.0, 1852.0, 954.0 ],
        "openrect": [ 0.0, 0.0, 640.2421875, 427.6627076966688 ],
        "openrectmode": 0,
        "openinpresentation": 1,
        "boxes": [
            {
                "box": {
                    "id": "obj-122",
                    "linecount": 2,
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1501.0, 288.5, 86.0, 33.0 ],
                    "text": "chance fade ramptime (ms)"
                }
            },
            {
                "box": {
                    "id": "obj-104",
                    "maxclass": "number",
                    "minimum": 0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1519.0, 328.0, 50.0, 22.0 ],
                    "varname": "ramptime-"
                }
            },
            {
                "box": {
                    "id": "obj-26",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2121.0, 208.0, 54.0, 22.0 ],
                    "text": "deferlow"
                }
            },
            {
                "box": {
                    "id": "obj-37",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2121.0, 235.0, 48.0, 22.0 ],
                    "text": "s DUMP"
                }
            },
            {
                "box": {
                    "id": "obj-17",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 1263.0, 294.0, 33.0, 22.0 ],
                    "text": "* 0.9"
                }
            },
            {
                "box": {
                    "allowdrag": 0,
                    "bgcolor": [ 0.125, 0.125, 0.125, 0.0 ],
                    "buffername": "buf",
                    "fontsize": 12.0,
                    "gridcolor": [ 0.221327066888467, 0.221327006361825, 0.221327022178404, 0.0 ],
                    "id": "obj-7",
                    "labels": 0,
                    "maxclass": "waveform~",
                    "numinlets": 5,
                    "numoutlets": 6,
                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                    "patching_rect": [ 1629.0, 256.0, 443.3648351430893, 117.41891896724701 ],
                    "presentation": 1,
                    "presentation_rect": [ 15.000000715255737, 158.99174785614014, 474.51050782203674, 111.56382060050964 ],
                    "ruler": 0,
                    "selectioncolor": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                    "setmode": 2,
                    "varname": "waveform",
                    "vticks": 0
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-321",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1435.6806721687317, 177.73108184337616, 47.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 180.0, 289.0, 47.0, 20.0 ],
                    "text": "chance"
                }
            },
            {
                "box": {
                    "bgcolor": [ 0.125, 0.125, 0.125, 0.0 ],
                    "id": "obj-272",
                    "maxclass": "multislider",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "orientation": 0,
                    "outlettype": [ "", "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1629.0, 255.0, 443.0, 120.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 15.000000715255737, 159.54730343818665, 474.0, 110.0 ],
                    "setminmax": [ 0.0, 1.0 ],
                    "size": 8,
                    "slidercolor": [ 1.0, 1.0, 1.0, 0.6 ],
                    "spacing": 2,
                    "thickness": 4,
                    "varname": "multislider-grainpos"
                }
            },
            {
                "box": {
                    "id": "obj-320",
                    "maxclass": "newobj",
                    "numinlets": 6,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 1434.0, 240.0, 101.0, 22.0 ],
                    "text": "scale 0 100 1. -1."
                }
            },
            {
                "box": {
                    "id": "obj-319",
                    "maxclass": "number",
                    "maximum": 100,
                    "minimum": 0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1434.0, 198.73948395252228, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 126.0, 288.0, 50.0, 22.0 ],
                    "varname": "chance-"
                }
            },
            {
                "box": {
                    "id": "obj-316",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1434.0, 328.0, 45.0, 22.0 ],
                    "text": "sig~ -1"
                }
            },
            {
                "box": {
                    "id": "obj-311",
                    "linecount": 3,
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1615.0, 555.0, 143.0, 47.0 ],
                    "text": " NOTES:\n\n- implement quantize etc."
                }
            },
            {
                "box": {
                    "id": "obj-308",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 922.0, 754.0, 52.0, 22.0 ],
                    "text": "s voices"
                }
            },
            {
                "box": {
                    "id": "obj-307",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 714.0, 317.0, 545.0, 458.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "id": "obj-27",
                                    "maxclass": "number",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 441.0, 281.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-322",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 441.0, 322.0, 67.0, 22.0 ],
                                    "text": "spacing $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-17",
                                    "maxclass": "number",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 350.0, 281.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-15",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 350.0, 322.0, 75.0, 22.0 ],
                                    "text": "thickness $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-13",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 218.0, 198.0, 49.0, 22.0 ],
                                    "text": "r length"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-8",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 24.0, 327.0, 47.0, 22.0 ],
                                    "text": "size $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-6",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 82.0, 327.0, 58.0, 22.0 ],
                                    "text": "set $1 $2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-5",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 82.0, 397.0, 221.0, 22.0 ],
                                    "text": "pattrforward parent::multislider-grainpos"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-293",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 82.0, 290.0, 57.0, 22.0 ],
                                    "text": "pack 0 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-297",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 120.0, 249.0, 29.5, 22.0 ],
                                    "text": "/ 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-298",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 120.0, 96.0, 50.0, 22.0 ],
                                    "text": "r voices",
                                    "varname": "voices"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-300",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 218.0, 249.0, 47.0, 22.0 ],
                                    "text": "size $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-301",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "float", "bang" ],
                                    "patching_rect": [ 218.0, 290.0, 95.0, 22.0 ],
                                    "text": "buffer~ grainpos"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-302",
                                    "maxclass": "toggle",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 99.0, 29.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-303",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "bang" ],
                                    "patching_rect": [ 99.0, 66.0, 113.0, 22.0 ],
                                    "text": "metro 20 @active 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-304",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 3,
                                    "outlettype": [ "bang", "bang", "int" ],
                                    "patching_rect": [ 99.0, 148.0, 40.0, 22.0 ],
                                    "text": "uzi 8"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-305",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 120.0, 198.0, 91.0, 22.0 ],
                                    "text": "peek~ grainpos"
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-297", 1 ],
                                    "midpoints": [ 227.5, 234.0, 140.0, 234.0 ],
                                    "order": 1,
                                    "source": [ "obj-13", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-300", 0 ],
                                    "midpoints": [ 227.5, 222.0, 227.5, 222.0 ],
                                    "order": 0,
                                    "source": [ "obj-13", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-5", 0 ],
                                    "midpoints": [ 359.5, 384.0, 91.5, 384.0 ],
                                    "source": [ "obj-15", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-15", 0 ],
                                    "midpoints": [ 359.5, 306.0, 359.5, 306.0 ],
                                    "source": [ "obj-17", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-322", 0 ],
                                    "midpoints": [ 450.5, 306.0, 450.5, 306.0 ],
                                    "source": [ "obj-27", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-6", 0 ],
                                    "midpoints": [ 91.5, 315.0, 91.5, 315.0 ],
                                    "source": [ "obj-293", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-293", 1 ],
                                    "midpoints": [ 129.5, 273.0, 129.5, 273.0 ],
                                    "source": [ "obj-297", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-304", 1 ],
                                    "midpoints": [ 129.5, 120.0, 129.5, 120.0 ],
                                    "order": 0,
                                    "source": [ "obj-298", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-8", 0 ],
                                    "midpoints": [ 129.5, 135.0, 33.5, 135.0 ],
                                    "order": 1,
                                    "source": [ "obj-298", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-301", 0 ],
                                    "midpoints": [ 227.5, 273.0, 227.5, 273.0 ],
                                    "source": [ "obj-300", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-303", 0 ],
                                    "midpoints": [ 108.5, 54.0, 108.5, 54.0 ],
                                    "source": [ "obj-302", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-304", 0 ],
                                    "midpoints": [ 108.5, 90.0, 108.5, 90.0 ],
                                    "source": [ "obj-303", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-293", 0 ],
                                    "midpoints": [ 129.5, 183.0, 91.5, 183.0 ],
                                    "order": 1,
                                    "source": [ "obj-304", 2 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-305", 0 ],
                                    "midpoints": [ 129.5, 171.0, 129.5, 171.0 ],
                                    "order": 0,
                                    "source": [ "obj-304", 2 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-297", 0 ],
                                    "midpoints": [ 129.5, 222.0, 129.5, 222.0 ],
                                    "source": [ "obj-305", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-5", 0 ],
                                    "midpoints": [ 450.5, 384.0, 91.5, 384.0 ],
                                    "source": [ "obj-322", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-5", 0 ],
                                    "midpoints": [ 91.5, 351.0, 91.5, 351.0 ],
                                    "source": [ "obj-6", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-5", 0 ],
                                    "midpoints": [ 33.5, 384.0, 91.5, 384.0 ],
                                    "source": [ "obj-8", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 1918.0, 208.0, 125.0, 22.0 ],
                    "text": "p multislider–grainpos"
                }
            },
            {
                "box": {
                    "floatoutput": 1,
                    "id": "obj-189",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 469.0, 475.0, 140.0, 20.0 ],
                    "size": 1.0
                }
            },
            {
                "box": {
                    "id": "obj-188",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "multichannelsignal" ],
                    "patching_rect": [ 1037.0, 523.0, 76.0, 22.0 ],
                    "text": "mc.dcblock~"
                }
            },
            {
                "box": {
                    "id": "obj-184",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "multichannelsignal" ],
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 59.0, 110.0, 216.0, 297.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "fontname": "Arial",
                                    "fontsize": 12.0,
                                    "id": "obj-174",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "multichannelsignal" ],
                                    "patching_rect": [ 50.0, 134.0, 39.0, 22.0 ],
                                    "text": "mc.-~"
                                }
                            },
                            {
                                "box": {
                                    "fontname": "Arial",
                                    "fontsize": 12.0,
                                    "id": "obj-177",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "multichannelsignal" ],
                                    "patching_rect": [ 70.0, 100.0, 94.0, 22.0 ],
                                    "text": "mc.onepole~ 10"
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-180",
                                    "index": 1,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "multichannelsignal" ],
                                    "patching_rect": [ 50.0, 36.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-182",
                                    "index": 1,
                                    "maxclass": "outlet",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 50.0, 216.0, 30.0, 30.0 ]
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-182", 0 ],
                                    "midpoints": [ 59.5, 159.0, 59.5, 159.0 ],
                                    "source": [ "obj-174", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-174", 1 ],
                                    "midpoints": [ 79.5, 123.0, 79.5, 123.0 ],
                                    "source": [ "obj-177", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-174", 0 ],
                                    "midpoints": [ 59.5, 69.0, 59.5, 69.0 ],
                                    "order": 1,
                                    "source": [ "obj-180", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-177", 0 ],
                                    "midpoints": [ 59.5, 87.0, 79.5, 87.0 ],
                                    "order": 0,
                                    "source": [ "obj-180", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 922.0, 523.0, 103.0, 22.0 ],
                    "text": "p \"DC offset filter\""
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-133",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 40.0, 715.0, 29.5, 22.0 ],
                    "text": "*~"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-162",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1686.0, 72.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 126.0, 393.0, 50.0, 22.0 ],
                    "varname": "buflength-"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-161",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1628.0, 72.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 16.0, 393.0, 50.0, 22.0 ],
                    "varname": "start-"
                }
            },
            {
                "box": {
                    "id": "obj-102",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 3123.0, 514.0, 150.0, 20.0 ]
                }
            },
            {
                "box": {
                    "border": 2.0,
                    "id": "obj-192",
                    "maxclass": "live.line",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 469.0, 552.0, 150.0, 6.0 ]
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-81",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 612.5, 660.0, 90.0, 20.0 ],
                    "text": "phasor position"
                }
            },
            {
                "box": {
                    "floatoutput": 1,
                    "id": "obj-95",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 469.0, 660.0, 140.0, 20.0 ],
                    "size": 1.0
                }
            },
            {
                "box": {
                    "id": "obj-54",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 469.0, 603.0, 81.0, 22.0 ],
                    "text": "snapshot~ 33"
                }
            },
            {
                "box": {
                    "id": "obj-73",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 469.0, 575.0, 94.0, 22.0 ],
                    "text": "receive~ phasor"
                }
            },
            {
                "box": {
                    "id": "obj-75",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "", "int", "int" ],
                    "patching_rect": [ 469.0, 629.0, 61.0, 22.0 ],
                    "text": "change 0."
                }
            },
            {
                "box": {
                    "id": "obj-4",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2349.0, 33.0, 79.0, 22.0 ],
                    "text": "r load-default"
                }
            },
            {
                "box": {
                    "id": "obj-227",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "", "", "" ],
                    "patching_rect": [ 2768.0, 402.0, 288.0, 22.0 ],
                    "saved_object_attributes": {
                        "initial": [ 1.0 ],
                        "parameter_enable": 0,
                        "parameter_mappable": 0
                    },
                    "text": "pattr gain-in @bindto gain-in- @initial 1. @invisible 1",
                    "varname": "gain-in"
                }
            },
            {
                "box": {
                    "id": "obj-218",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2879.0, 38.0, 67.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 529.8744353950024, 14.0, 67.0, 20.0 ],
                    "text": "audio input"
                }
            },
            {
                "box": {
                    "id": "obj-205",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 214.0, 574.0, 82.0, 22.0 ],
                    "text": "send~ phasor"
                }
            },
            {
                "box": {
                    "id": "obj-194",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1015.0, 460.0, 31.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 531.5071583688259, 323.0833334326744, 31.0, 20.0 ],
                    "text": "fuzz"
                }
            },
            {
                "box": {
                    "id": "obj-193",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 948.0, 460.0, 35.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 529.6460466682911, 288.00000005960464, 35.0, 20.0 ],
                    "text": "mute"
                }
            },
            {
                "box": {
                    "id": "obj-166",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 989.0, 426.0, 38.0, 22.0 ],
                    "text": "r fuzz"
                }
            },
            {
                "box": {
                    "id": "obj-155",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 922.0, 426.0, 43.0, 22.0 ],
                    "text": "r mute"
                }
            },
            {
                "box": {
                    "id": "obj-167",
                    "maxclass": "toggle",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 989.0, 458.0, 24.0, 24.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 501.64604583382607, 321.0, 24.0, 24.0 ]
                }
            },
            {
                "box": {
                    "id": "obj-168",
                    "maxclass": "toggle",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 922.0, 458.0, 24.0, 24.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 501.64604583382607, 286.0, 24.0, 24.0 ]
                }
            },
            {
                "box": {
                    "activebgcolor": [ 0.156862745098039, 0.156862745098039, 0.156862745098039, 0.0 ],
                    "activegridcolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "activelinecolor": [ 0.618934978328545, 0.744701397656435, 0.953750108255376, 1.0 ],
                    "bgcolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "decay_time": 1.0,
                    "gridcolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-175",
                    "line_width": 2.0,
                    "linecolor": [ 0.325, 0.325, 0.325, 1.0 ],
                    "maxclass": "live.scope~",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "bang" ],
                    "patching_rect": [ 1129.0, 629.0, 82.0, 59.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 504.80000752210617, 161.20000240206718, 116.0000017285347, 106.00000157952309 ],
                    "range": [ -0.5, 0.5 ],
                    "rounded": 0.0,
                    "samples": 441.0,
                    "saved_attribute_attributes": {
                        "linecolor": {
                            "expression": "themecolor.live_lcd_control_fg_zombie"
                        }
                    },
                    "smooth": 2
                }
            },
            {
                "box": {
                    "id": "obj-88",
                    "maxclass": "newobj",
                    "numinlets": 4,
                    "numoutlets": 1,
                    "outlettype": [ "multichannelsignal" ],
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 109.0, 169.0, 413.0, 551.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "id": "obj-6",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 25.0, 83.0, 29.5, 22.0 ],
                                    "text": "!- 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-5",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "patching_rect": [ 147.0, 114.0, 40.0, 22.0 ],
                                    "text": "line"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-3",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 147.0, 83.0, 64.0, 22.0 ],
                                    "text": "pack f 500"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-4",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "multichannelsignal" ],
                                    "patching_rect": [ 268.0, 446.0, 80.0, 22.0 ],
                                    "text": "mc.combine~"
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-2",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 201.0, 217.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-67",
                                    "maxclass": "newobj",
                                    "numinlets": 6,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 147.0, 161.0, 90.0, 22.0 ],
                                    "text": "scale 0. 1. 1. 8."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-65",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 329.0, 375.0, 34.0, 22.0 ],
                                    "text": "*~ 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-66",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 268.0, 375.0, 34.0, 22.0 ],
                                    "text": "*~ 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-63",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 25.0, 217.0, 41.0, 22.0 ],
                                    "text": "sig~ 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-64",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 25.0, 261.0, 87.0, 22.0 ],
                                    "text": "slide~ 256 256"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-60",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 147.0, 217.0, 41.0, 22.0 ],
                                    "text": "sig~ 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-59",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 147.0, 261.0, 87.0, 22.0 ],
                                    "text": "slide~ 256 256"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-56",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 329.0, 410.0, 39.0, 22.0 ],
                                    "text": "atan~"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-54",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 268.0, 410.0, 39.0, 22.0 ],
                                    "text": "atan~"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-12",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 329.0, 308.0, 34.0, 22.0 ],
                                    "text": "*~ 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-13",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 268.0, 308.0, 34.0, 22.0 ],
                                    "text": "*~ 1."
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-68",
                                    "index": 1,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 25.0, 26.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-69",
                                    "index": 2,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 147.0, 26.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-70",
                                    "index": 3,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 268.0, 26.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-71",
                                    "index": 4,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 329.0, 26.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-72",
                                    "index": 1,
                                    "maxclass": "outlet",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 268.0, 493.0, 30.0, 30.0 ]
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-65", 0 ],
                                    "midpoints": [ 338.5, 333.0, 338.5, 333.0 ],
                                    "source": [ "obj-12", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-66", 0 ],
                                    "midpoints": [ 277.5, 333.0, 277.5, 333.0 ],
                                    "source": [ "obj-13", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-5", 0 ],
                                    "midpoints": [ 156.5, 108.0, 156.5, 108.0 ],
                                    "source": [ "obj-3", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-72", 0 ],
                                    "midpoints": [ 277.5, 471.0, 277.5, 471.0 ],
                                    "source": [ "obj-4", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-67", 0 ],
                                    "midpoints": [ 156.5, 138.0, 156.5, 138.0 ],
                                    "source": [ "obj-5", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-4", 0 ],
                                    "midpoints": [ 277.5, 435.0, 277.5, 435.0 ],
                                    "source": [ "obj-54", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-4", 1 ],
                                    "midpoints": [ 338.5, 435.0, 338.5, 435.0 ],
                                    "source": [ "obj-56", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-12", 1 ],
                                    "midpoints": [ 156.5, 294.0, 353.5, 294.0 ],
                                    "order": 0,
                                    "source": [ "obj-59", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-13", 1 ],
                                    "midpoints": [ 156.5, 294.0, 292.5, 294.0 ],
                                    "order": 1,
                                    "source": [ "obj-59", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-63", 0 ],
                                    "source": [ "obj-6", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-59", 0 ],
                                    "midpoints": [ 156.5, 240.0, 156.5, 240.0 ],
                                    "source": [ "obj-60", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-64", 0 ],
                                    "midpoints": [ 34.5, 240.0, 34.5, 240.0 ],
                                    "source": [ "obj-63", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-65", 1 ],
                                    "midpoints": [ 34.5, 360.0, 353.5, 360.0 ],
                                    "order": 0,
                                    "source": [ "obj-64", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-66", 1 ],
                                    "midpoints": [ 34.5, 360.0, 292.5, 360.0 ],
                                    "order": 1,
                                    "source": [ "obj-64", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-56", 0 ],
                                    "midpoints": [ 338.5, 399.0, 338.5, 399.0 ],
                                    "source": [ "obj-65", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-54", 0 ],
                                    "midpoints": [ 277.5, 399.0, 277.5, 399.0 ],
                                    "source": [ "obj-66", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-2", 0 ],
                                    "midpoints": [ 156.5, 204.0, 210.5, 204.0 ],
                                    "order": 0,
                                    "source": [ "obj-67", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-60", 0 ],
                                    "midpoints": [ 156.5, 186.0, 156.5, 186.0 ],
                                    "order": 1,
                                    "source": [ "obj-67", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-6", 0 ],
                                    "source": [ "obj-68", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-3", 0 ],
                                    "midpoints": [ 156.5, 57.0, 156.5, 57.0 ],
                                    "source": [ "obj-69", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-13", 0 ],
                                    "midpoints": [ 277.5, 57.0, 277.5, 57.0 ],
                                    "source": [ "obj-70", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-12", 0 ],
                                    "midpoints": [ 338.5, 57.0, 338.5, 57.0 ],
                                    "source": [ "obj-71", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 922.0, 493.0, 223.0, 22.0 ],
                    "text": "p fx"
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "bgcolor": [ 0.125490196078431, 0.125490196078431, 0.125490196078431, 1.0 ],
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-178",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1128.0, 629.0, 83.0, 59.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 501.64604583382607, 157.61315467953682, 123.4567791223526, 112.75719159841537 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "id": "obj-150",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "bang", "float" ],
                    "patching_rect": [ 566.8066889047623, 92.85713732242584, 29.5, 22.0 ],
                    "text": "t b f"
                }
            },
            {
                "box": {
                    "id": "obj-59",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1042.0, 58.5, 33.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 252.0, 118.0, 33.0, 20.0 ],
                    "text": "rand"
                }
            },
            {
                "box": {
                    "id": "obj-12",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2349.0, 62.0, 45.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 305.0, 117.0, 45.0, 22.0 ],
                    "text": "default"
                }
            },
            {
                "box": {
                    "id": "obj-214",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 2321.0, 408.0, 47.0, 22.0 ],
                    "text": "* 1000."
                }
            },
            {
                "box": {
                    "floatoutput": 1,
                    "id": "obj-241",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 671.008363366127, 47.89915680885315, 140.0, 20.0 ],
                    "size": 1.0
                }
            },
            {
                "box": {
                    "id": "obj-173",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 59.0, 110.0, 197.0, 405.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-9",
                                    "index": 2,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 127.0, 38.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-6",
                                    "index": 1,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 38.0, 38.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-7",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "float" ],
                                    "patching_rect": [ 127.0, 97.0, 29.5, 22.0 ],
                                    "text": "t b f"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-4",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 38.0, 198.0, 55.0, 22.0 ],
                                    "text": "pak 0. 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-46",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 74.0, 153.0, 82.0, 22.0 ],
                                    "text": "expr $f1 + $f2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-169",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "" ],
                                    "patching_rect": [ 38.0, 237.0, 51.0, 22.0 ],
                                    "text": "zl.sort 1"
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-170",
                                    "index": 1,
                                    "maxclass": "outlet",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 38.0, 336.0, 30.0, 30.0 ]
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-170", 0 ],
                                    "source": [ "obj-169", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-169", 0 ],
                                    "midpoints": [ 47.5, 222.0, 47.5, 222.0 ],
                                    "source": [ "obj-4", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-4", 1 ],
                                    "source": [ "obj-46", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-4", 0 ],
                                    "midpoints": [ 47.5, 69.0, 47.5, 69.0 ],
                                    "order": 1,
                                    "source": [ "obj-6", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-46", 0 ],
                                    "midpoints": [ 47.5, 138.0, 83.5, 138.0 ],
                                    "order": 0,
                                    "source": [ "obj-6", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-46", 1 ],
                                    "midpoints": [ 147.0, 150.0, 146.5, 150.0 ],
                                    "source": [ "obj-7", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-46", 0 ],
                                    "midpoints": [ 136.5, 138.0, 83.5, 138.0 ],
                                    "source": [ "obj-7", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-7", 0 ],
                                    "midpoints": [ 136.5, 69.0, 136.5, 69.0 ],
                                    "source": [ "obj-9", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 1628.0, 130.0, 68.0, 22.0 ],
                    "text": "p buflength"
                }
            },
            {
                "box": {
                    "floatoutput": 1,
                    "id": "obj-165",
                    "listmode": 1,
                    "maxclass": "rslider",
                    "numinlets": 2,
                    "numoutlets": 2,
                    "outlettype": [ "", "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1628.0, 177.0, 134.85714888572693, 23.28571917116642 ],
                    "size": 1.0
                }
            },
            {
                "box": {
                    "floatoutput": 1,
                    "id": "obj-147",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 185.0, 475.0, 140.0, 20.0 ],
                    "size": 1.0
                }
            },
            {
                "box": {
                    "id": "obj-92",
                    "linecount": 2,
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "multichannelsignal" ],
                    "patching_rect": [ 3163.0, 221.0, 77.0, 35.0 ],
                    "text": "mc.receive~ master 2"
                }
            },
            {
                "box": {
                    "id": "obj-79",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1000.0, 666.0, 101.0, 22.0 ],
                    "text": "mc.send~ master"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-30",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 612.0, 475.0, 91.0, 20.0 ],
                    "text": "playing position"
                }
            },
            {
                "box": {
                    "id": "obj-16",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 1918.0, 108.0, 70.0, 22.0 ],
                    "text": "loadmess 0"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-114",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1947.0, 141.0, 121.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 70.0, 118.0, 121.0, 20.0 ],
                    "text": "enable mouse (pitch)"
                }
            },
            {
                "box": {
                    "id": "obj-113",
                    "maxclass": "toggle",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1918.0, 140.0, 24.0, 24.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 42.0, 116.0, 24.0, 24.0 ],
                    "varname": "toggle[1]"
                }
            },
            {
                "box": {
                    "id": "obj-66",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 649.0, 437.0, 54.0, 22.0 ],
                    "text": "s curpos"
                }
            },
            {
                "box": {
                    "id": "obj-61",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 59.0, 110.0, 537.0, 211.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "id": "obj-8",
                                    "maxclass": "toggle",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 184.0, 79.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-6",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 184.0, 114.0, 61.0, 22.0 ],
                                    "text": "hidden $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-206",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 37.0, 114.0, 41.0, 22.0 ],
                                    "text": "in0 $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-194",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 90.0, 114.0, 41.0, 22.0 ],
                                    "text": "in1 $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-192",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 90.0, 164.0, 149.0, 22.0 ],
                                    "text": "pattrforward parent::rslider"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-188",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 90.0, 31.0, 52.0, 22.0 ],
                                    "text": "r curpos",
                                    "varname": "u101000646"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-178",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 157.0, 31.0, 245.0, 22.0 ],
                                    "restore": [ 128.9654652800119 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr @bindto parent::grainsize- @invisible 1",
                                    "varname": "u322010437[1]"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-102",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 90.0, 80.0, 29.5, 22.0 ],
                                    "text": "+ 0"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-99",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 431.0, 80.0, 29.5, 22.0 ],
                                    "text": "+ 1"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-98",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 431.0, 31.0, 49.0, 22.0 ],
                                    "text": "r length"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-101",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 431.0, 114.0, 66.0, 22.0 ],
                                    "text": "in0 size $1"
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-192", 0 ],
                                    "midpoints": [ 440.5, 150.0, 99.5, 150.0 ],
                                    "source": [ "obj-101", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-194", 0 ],
                                    "midpoints": [ 99.5, 105.0, 99.5, 105.0 ],
                                    "source": [ "obj-102", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-102", 1 ],
                                    "midpoints": [ 166.5, 66.0, 110.0, 66.0 ],
                                    "source": [ "obj-178", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-102", 0 ],
                                    "midpoints": [ 99.5, 54.0, 99.5, 54.0 ],
                                    "order": 0,
                                    "source": [ "obj-188", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-206", 0 ],
                                    "midpoints": [ 99.5, 66.0, 46.5, 66.0 ],
                                    "order": 1,
                                    "source": [ "obj-188", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-192", 0 ],
                                    "midpoints": [ 99.5, 138.0, 99.5, 138.0 ],
                                    "source": [ "obj-194", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-192", 0 ],
                                    "midpoints": [ 46.5, 150.0, 99.5, 150.0 ],
                                    "source": [ "obj-206", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-192", 0 ],
                                    "midpoints": [ 193.5, 150.0, 99.5, 150.0 ],
                                    "source": [ "obj-6", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-6", 0 ],
                                    "source": [ "obj-8", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-99", 0 ],
                                    "midpoints": [ 440.5, 54.0, 440.5, 54.0 ],
                                    "source": [ "obj-98", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-101", 0 ],
                                    "midpoints": [ 440.5, 105.0, 440.5, 105.0 ],
                                    "source": [ "obj-99", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 1780.0, 208.0, 106.0, 22.0 ],
                    "text": "p rslider–grainsize",
                    "varname": "rslider[1]"
                }
            },
            {
                "box": {
                    "id": "obj-43",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 533.0, 178.0, 252.0, 352.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "id": "obj-8",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 47.0, 78.0, 41.0, 22.0 ],
                                    "text": "unjoin"
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-5",
                                    "index": 1,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 47.0, 29.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-2",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 47.0, 254.0, 72.0, 22.0 ],
                                    "text": "prepend in2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-1",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 47.0, 294.0, 169.0, 22.0 ],
                                    "text": "pattrforward parent::waveform"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-134",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "" ],
                                    "patching_rect": [ 47.0, 194.0, 51.0, 22.0 ],
                                    "text": "zl.sort 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-133",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 47.0, 167.0, 74.0, 22.0 ],
                                    "text": "pak 0. 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-229",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 102.0, 138.0, 29.5, 22.0 ],
                                    "text": "* 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-218",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 47.0, 138.0, 29.5, 22.0 ],
                                    "text": "* 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-75",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 147.0, 29.0, 49.0, 22.0 ],
                                    "text": "r length"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-74",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 147.0, 194.0, 35.0, 22.0 ],
                                    "text": "0. $1"
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-134", 0 ],
                                    "midpoints": [ 56.5, 192.0, 56.5, 192.0 ],
                                    "source": [ "obj-133", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-2", 0 ],
                                    "midpoints": [ 56.5, 219.0, 56.5, 219.0 ],
                                    "source": [ "obj-134", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-1", 0 ],
                                    "source": [ "obj-2", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-133", 0 ],
                                    "midpoints": [ 56.5, 162.0, 56.5, 162.0 ],
                                    "source": [ "obj-218", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-133", 1 ],
                                    "midpoints": [ 111.5, 162.0, 111.5, 162.0 ],
                                    "source": [ "obj-229", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-8", 0 ],
                                    "midpoints": [ 56.5, 60.0, 56.5, 60.0 ],
                                    "source": [ "obj-5", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-2", 0 ],
                                    "midpoints": [ 156.5, 240.0, 56.5, 240.0 ],
                                    "source": [ "obj-74", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-218", 1 ],
                                    "midpoints": [ 156.5, 123.0, 67.0, 123.0 ],
                                    "order": 2,
                                    "source": [ "obj-75", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-229", 1 ],
                                    "midpoints": [ 156.5, 123.0, 122.0, 123.0 ],
                                    "order": 1,
                                    "source": [ "obj-75", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-74", 0 ],
                                    "order": 0,
                                    "source": [ "obj-75", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-218", 0 ],
                                    "midpoints": [ 56.5, 102.0, 56.5, 102.0 ],
                                    "source": [ "obj-8", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-229", 0 ],
                                    "midpoints": [ 67.5, 123.0, 111.5, 123.0 ],
                                    "source": [ "obj-8", 1 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 1628.0, 208.0, 71.0, 22.0 ],
                    "text": "p waveform"
                }
            },
            {
                "box": {
                    "fontname": "Tahoma",
                    "fontsize": 12.0,
                    "id": "obj-139",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2959.0, 124.0, 61.0, 21.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 556.1451074779034, 87.0, 61.0, 21.0 ],
                    "text": "input gain"
                }
            },
            {
                "box": {
                    "id": "obj-71",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 2,
                    "outlettype": [ "bang", "" ],
                    "patching_rect": [ 3115.0, 76.0, 34.0, 22.0 ],
                    "text": "sel 1"
                }
            },
            {
                "box": {
                    "id": "obj-262",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "int", "" ],
                    "patching_rect": [ 3115.0, 221.0, 29.5, 22.0 ],
                    "text": "t 1 l"
                }
            },
            {
                "box": {
                    "id": "obj-261",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 3163.0, 143.0, 29.5, 22.0 ],
                    "text": "0"
                }
            },
            {
                "box": {
                    "id": "obj-259",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 3115.0, 193.0, 83.0, 22.0 ],
                    "text": "prepend open"
                }
            },
            {
                "box": {
                    "id": "obj-258",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "patching_rect": [ 3115.0, 108.0, 67.0, 22.0 ],
                    "text": "opendialog"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-255",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 2876.0, 146.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 501.5878102183342, 86.0, 50.0, 22.0 ],
                    "varname": "gain-in-"
                }
            },
            {
                "box": {
                    "id": "obj-253",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2614.0, 63.0, 70.0, 22.0 ],
                    "text": "loadmess 1"
                }
            },
            {
                "box": {
                    "id": "obj-252",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2876.0, 63.0, 73.0, 22.0 ],
                    "text": "loadmess 1."
                }
            },
            {
                "box": {
                    "floatoutput": 1,
                    "id": "obj-249",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 2876.0, 102.0, 140.0, 20.0 ],
                    "size": 2.0
                }
            },
            {
                "box": {
                    "id": "obj-217",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 206.0, 335.0, 65.0, 22.0 ],
                    "text": "r buflength"
                }
            },
            {
                "box": {
                    "id": "obj-215",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "bang", "float" ],
                    "patching_rect": [ 206.0, 369.0, 29.5, 22.0 ],
                    "text": "t b f"
                }
            },
            {
                "box": {
                    "id": "obj-210",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 139.0, 437.0, 29.5, 22.0 ],
                    "text": "/ 1."
                }
            },
            {
                "box": {
                    "id": "obj-138",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 152.0, 619.0, 39.0, 22.0 ],
                    "text": "r start"
                }
            },
            {
                "box": {
                    "id": "obj-131",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 671.008363366127, 170.16805708408356, 29.5, 22.0 ],
                    "text": "/ 2."
                }
            },
            {
                "box": {
                    "id": "obj-235",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1962.0, 490.0, 87.0, 22.0 ],
                    "text": "s buflength-ms"
                }
            },
            {
                "box": {
                    "id": "obj-233",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1841.0, 432.0, 67.0, 22.0 ],
                    "text": "s buflength"
                }
            },
            {
                "box": {
                    "id": "obj-231",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1721.0, 432.0, 41.0, 22.0 ],
                    "text": "s start"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-220",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1962.0, 432.0, 50.0, 22.0 ]
                }
            },
            {
                "box": {
                    "id": "obj-223",
                    "linecount": 2,
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1983.0, 456.0, 69.0, 33.0 ],
                    "text": "buf\nlength (ms)"
                }
            },
            {
                "box": {
                    "id": "obj-224",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1637.0, 47.0, 32.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 70.0000008046627, 393.80000001192093, 32.0, 20.0 ],
                    "text": "start"
                }
            },
            {
                "box": {
                    "id": "obj-199",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1681.0, 47.0, 61.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 180.0, 394.0, 61.0, 20.0 ],
                    "text": "buf length"
                }
            },
            {
                "box": {
                    "id": "obj-225",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 3,
                    "outlettype": [ "float", "float", "float" ],
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 157.0, 252.0, 369.0, 388.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "id": "obj-8",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 277.0, 79.0, 54.0, 22.0 ],
                                    "text": "deferlow"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-5",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "float" ],
                                    "patching_rect": [ 277.0, 186.0, 29.5, 22.0 ],
                                    "text": "t b f"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-1",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "float" ],
                                    "patching_rect": [ 168.0, 139.0, 29.5, 22.0 ],
                                    "text": "t b f"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-15",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 257.0, 347.0, 69.0, 20.0 ],
                                    "text": "length (ms)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-14",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 152.0, 347.0, 77.0, 20.0 ],
                                    "text": "length (0 - 1)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-13",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 25.0, 347.0, 67.0, 20.0 ],
                                    "text": "start (0 - 1)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-4",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 264.0, 20.0, 75.0, 20.0 ],
                                    "text": "buffer length"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-3",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 144.0, 20.0, 79.0, 20.0 ],
                                    "text": "selection end"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-2",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 17.0, 20.0, 83.0, 20.0 ],
                                    "text": "selection start"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-126",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 168.0, 259.0, 29.5, 22.0 ],
                                    "text": "/ 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-122",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 168.0, 186.0, 29.5, 22.0 ],
                                    "text": "!- 1."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-121",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 39.0, 259.0, 29.5, 22.0 ],
                                    "text": "/ 1."
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-169",
                                    "index": 1,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 39.0, 47.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-170",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 277.0, 47.0, 49.0, 22.0 ],
                                    "text": "r length"
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-171",
                                    "index": 2,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 168.0, 47.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-172",
                                    "index": 1,
                                    "maxclass": "outlet",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 38.0, 308.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-173",
                                    "index": 3,
                                    "maxclass": "outlet",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 277.0, 308.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-174",
                                    "index": 2,
                                    "maxclass": "outlet",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 168.0, 308.0, 30.0, 30.0 ]
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-122", 1 ],
                                    "midpoints": [ 188.0, 162.0, 188.0, 162.0 ],
                                    "source": [ "obj-1", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-122", 0 ],
                                    "midpoints": [ 177.5, 162.0, 177.5, 162.0 ],
                                    "source": [ "obj-1", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-172", 0 ],
                                    "midpoints": [ 48.5, 282.0, 47.5, 282.0 ],
                                    "source": [ "obj-121", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-126", 0 ],
                                    "midpoints": [ 177.5, 210.0, 177.5, 210.0 ],
                                    "order": 1,
                                    "source": [ "obj-122", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-173", 0 ],
                                    "midpoints": [ 177.5, 246.0, 286.5, 246.0 ],
                                    "order": 0,
                                    "source": [ "obj-122", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-174", 0 ],
                                    "midpoints": [ 177.5, 282.0, 177.5, 282.0 ],
                                    "source": [ "obj-126", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-121", 0 ],
                                    "midpoints": [ 48.5, 78.0, 48.5, 78.0 ],
                                    "order": 1,
                                    "source": [ "obj-169", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-122", 0 ],
                                    "midpoints": [ 48.5, 171.0, 177.5, 171.0 ],
                                    "order": 0,
                                    "source": [ "obj-169", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-8", 0 ],
                                    "midpoints": [ 286.5, 72.0, 286.5, 72.0 ],
                                    "source": [ "obj-170", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-1", 0 ],
                                    "midpoints": [ 177.5, 78.0, 177.5, 78.0 ],
                                    "source": [ "obj-171", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-121", 1 ],
                                    "midpoints": [ 297.0, 246.0, 59.0, 246.0 ],
                                    "order": 1,
                                    "source": [ "obj-5", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-121", 0 ],
                                    "midpoints": [ 286.5, 246.0, 48.5, 246.0 ],
                                    "order": 1,
                                    "source": [ "obj-5", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-126", 1 ],
                                    "midpoints": [ 297.0, 246.0, 188.0, 246.0 ],
                                    "order": 0,
                                    "source": [ "obj-5", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-126", 0 ],
                                    "midpoints": [ 286.5, 246.0, 177.5, 246.0 ],
                                    "order": 0,
                                    "source": [ "obj-5", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-1", 0 ],
                                    "midpoints": [ 286.5, 126.0, 177.5, 126.0 ],
                                    "order": 1,
                                    "source": [ "obj-8", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-5", 0 ],
                                    "midpoints": [ 286.5, 102.0, 286.5, 102.0 ],
                                    "order": 0,
                                    "source": [ "obj-8", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 1721.0, 403.0, 260.0, 22.0 ],
                    "text": "p selection"
                }
            },
            {
                "box": {
                    "id": "obj-216",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 126.0, 574.0, 65.0, 22.0 ],
                    "text": "r buflength"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-130",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 77.0, 664.0, 29.5, 22.0 ],
                    "text": "+~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-127",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 77.0, 619.0, 29.5, 22.0 ],
                    "text": "*~"
                }
            },
            {
                "box": {
                    "id": "obj-213",
                    "maxclass": "number",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 922.0, 294.0, 50.0, 22.0 ],
                    "varname": "voices-"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-42",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 236.0, 438.0, 50.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 69.14685368537903, 289.0, 50.0, 20.0 ],
                    "text": "position"
                }
            },
            {
                "box": {
                    "id": "obj-41",
                    "lastchannelcount": 2,
                    "maxclass": "mc.live.gain~",
                    "numinlets": 1,
                    "numoutlets": 4,
                    "orientation": 1,
                    "outlettype": [ "multichannelsignal", "", "float", "list" ],
                    "parameter_enable": 1,
                    "patching_rect": [ 922.0, 554.0, 136.0, 47.0 ],
                    "saved_attribute_attributes": {
                        "valueof": {
                            "parameter_longname": "live.gain~",
                            "parameter_mmax": 6.0,
                            "parameter_mmin": -70.0,
                            "parameter_modmode": 3,
                            "parameter_shortname": "master",
                            "parameter_type": 0,
                            "parameter_unitstyle": 4
                        }
                    },
                    "varname": "live.gain~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-5",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 727.5, 220.0, 121.0, 20.0 ],
                    "text": "random position (ms)"
                }
            },
            {
                "box": {
                    "id": "obj-160",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1016.0, 116.5, 42.0, 22.0 ],
                    "text": "s rand"
                }
            },
            {
                "box": {
                    "id": "obj-159",
                    "maxclass": "button",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1016.0, 56.5, 24.0, 24.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 223.0, 116.0, 24.0, 24.0 ]
                }
            },
            {
                "box": {
                    "id": "obj-187",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "", "", "" ],
                    "patching_rect": [ 922.0, 721.0, 280.0, 22.0 ],
                    "saved_object_attributes": {
                        "initial": [ 8 ],
                        "parameter_enable": 0,
                        "parameter_mappable": 0
                    },
                    "text": "pattr voices @bindto voices- @initial 8 @invisible 1",
                    "varname": "voices"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-151",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 1176.0, 58.0, 89.0, 22.0 ],
                    "text": "storagewindow"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-153",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 1093.0, 58.0, 63.0, 22.0 ],
                    "text": "writeagain"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-164",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 1093.0, 116.0, 353.0, 22.0 ],
                    "saved_object_attributes": {
                        "client_rect": [ 4, 45, 358, 173 ],
                        "parameter_enable": 0,
                        "parameter_mappable": 0,
                        "storage_rect": [ 583, 69, 1034, 197 ]
                    },
                    "text": "pattrstorage granulator @greedy 1 @changemode 1 @savemode 2",
                    "varname": "granulator"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-57",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2375.0, 444.0, 69.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 556.0, 118.0, 69.0, 20.0 ],
                    "text": "length (ms)"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-89",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 2321.0, 443.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 501.5878102183342, 117.0, 50.0, 22.0 ],
                    "varname": "number[13]"
                }
            },
            {
                "box": {
                    "id": "obj-186",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 922.0, 666.0, 54.0, 22.0 ],
                    "text": "mc.dac~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-185",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 469.0, 329.0, 81.0, 22.0 ],
                    "text": "snapshot~ 33"
                }
            },
            {
                "box": {
                    "id": "obj-183",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 469.0, 279.0, 99.0, 22.0 ],
                    "text": "receive~ position"
                }
            },
            {
                "box": {
                    "id": "obj-181",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 40.0, 751.0, 86.0, 22.0 ],
                    "text": "send~ position"
                }
            },
            {
                "box": {
                    "id": "obj-176",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 566.8066889047623, 47.89915680885315, 85.0, 22.0 ],
                    "text": "r buflength-ms"
                }
            },
            {
                "box": {
                    "id": "obj-172",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2018.0, 403.0, 54.0, 22.0 ],
                    "text": "s mouse"
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-171",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2614.0, 38.0, 168.0, 20.0 ],
                    "text": "record audio input into buffer~"
                }
            },
            {
                "box": {
                    "fontsize": 12.0,
                    "id": "obj-158",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 34.0, 91.0, 1372.0, 807.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "id": "obj-145",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 1587.4753784537315, 1051.9380007982254, 298.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "initial": [ 100 ],
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr ramptime @bindto parent::ramptime- @initial 100",
                                    "varname": "ramptime"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-140",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1908.527161359787, 1005.4263721704483, 107.0, 22.0 ],
                                    "text": "expr int($f1 * 100.)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-139",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 1908.527161359787, 1051.9380007982254, 276.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "initial": [ 100 ],
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr chance @bindto parent::chance- @initial 100",
                                    "varname": "chance[1]"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-86",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1875.19382750988, 897.674432516098, 29.5, 22.0 ],
                                    "text": "+ 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-77",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1875.19382750988, 855.8139667510986, 33.0, 22.0 ],
                                    "text": "== 1"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-85",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1875.19382750988, 823.2558267116547, 35.0, 22.0 ],
                                    "text": "r P10"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-78",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "" ],
                                    "patching_rect": [ 1875.19382750988, 958.1395497322083, 52.0, 22.0 ],
                                    "text": "gate 2 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-20",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "patching_rect": [ 1213.0, 380.0, 51.0, 22.0 ],
                                    "text": "line 0. 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-72",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1213.0, 347.0, 57.0, 22.0 ],
                                    "text": "pack f 80"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-11",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1903.0, 675.0, 77.0, 22.0 ],
                                    "text": "loadmess 17"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-124",
                                    "maxclass": "led",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 581.0, 899.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-128",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patcher": {
                                        "fileversion": 1,
                                        "appversion": {
                                            "major": 9,
                                            "minor": 2,
                                            "revision": 0,
                                            "architecture": "x64",
                                            "modernui": 1
                                        },
                                        "classnamespace": "box",
                                        "rect": [ 59.0, 110.0, 168.0, 259.0 ],
                                        "boxes": [
                                            {
                                                "box": {
                                                    "id": "obj-99",
                                                    "maxclass": "newobj",
                                                    "numinlets": 2,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "" ],
                                                    "patching_rect": [ 94.47514718770981, 154.1436612010002, 55.0, 22.0 ],
                                                    "text": "pipe 800"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "id": "obj-91",
                                                    "maxclass": "newobj",
                                                    "numinlets": 1,
                                                    "numoutlets": 3,
                                                    "outlettype": [ "int", "int", "stop" ],
                                                    "patching_rect": [ 50.00002219737621, 104.0, 58.0, 22.0 ],
                                                    "text": "t 0 1 stop"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-132",
                                                    "index": 1,
                                                    "maxclass": "inlet",
                                                    "numinlets": 0,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "bang" ],
                                                    "patching_rect": [ 50.00002219737621, 40.00001193177411, 30.0, 30.0 ]
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-133",
                                                    "index": 1,
                                                    "maxclass": "outlet",
                                                    "numinlets": 1,
                                                    "numoutlets": 0,
                                                    "patching_rect": [ 69.50002219737621, 204.97239536046982, 30.0, 30.0 ]
                                                }
                                            }
                                        ],
                                        "lines": [
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-91", 0 ],
                                                    "midpoints": [ 59.50002219737621, 72.0, 59.50002219737621, 72.0 ],
                                                    "source": [ "obj-132", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-133", 0 ],
                                                    "midpoints": [ 79.00002219737621, 129.0, 79.00002219737621, 129.0 ],
                                                    "source": [ "obj-91", 1 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-99", 0 ],
                                                    "midpoints": [ 98.50002219737621, 141.0, 103.97514718770981, 141.0 ],
                                                    "source": [ "obj-91", 2 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-99", 0 ],
                                                    "midpoints": [ 59.50002219737621, 141.0, 103.97514718770981, 141.0 ],
                                                    "source": [ "obj-91", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-133", 0 ],
                                                    "midpoints": [ 103.97514718770981, 189.0, 79.00002219737621, 189.0 ],
                                                    "source": [ "obj-99", 0 ]
                                                }
                                            }
                                        ]
                                    },
                                    "patching_rect": [ 581.0, 870.0, 32.0, 22.0 ],
                                    "text": "p on"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-130",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 580.0, 932.0, 34.0, 22.0 ],
                                    "text": "s LED"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-132",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 581.0, 840.0, 34.0, 22.0 ],
                                    "text": "sel 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-133",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "int", "int" ],
                                    "patching_rect": [ 581.0, 807.0, 48.0, 22.0 ],
                                    "text": "change"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-137",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 581.0, 776.0, 43.0, 22.0 ],
                                    "text": "== 0.5"
                                }
                            },
                            {
                                "box": {
                                    "bubble": 1,
                                    "bubbletextmargin": 12,
                                    "fontsize": 12.0,
                                    "id": "obj-73",
                                    "linecount": 28,
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 13.0, 21.0, 294.0, 400.0 ],
                                    "presentation": 1,
                                    "presentation_linecount": 28,
                                    "presentation_rect": [ 682.0, 59.0, 294.0, 400.0 ],
                                    "text": "MAPPINGS\n\nknob 1 – playback start position\nknob 2 – grain size\nknob 3 – random position\nknob 4 – pitch\nknob 5 – random pitch\nknob 6 – playback end position\n\nswitch1 ON/OFF/ON – playback mode \n                                    (reverse/freeze/forwards)\n\nSWITCH1 ON (1/2):\nslider 1 – playback speed\nslider 2 – gain\n\nSWITCH1 OFF (0):\nslider 1 – playback position (phasor~)\n\nswitch2 ON   (1) – mute\n             OFF (0) – unmute\n             ON   (2) – fuzz (distortion)\n\ntouch – preset interpolation (?)\npress – 1 = store preset 1 (?)\n\np11 – record\np12 – randomize"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-60",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "patching_rect": [ 2025.0, 666.0, 51.0, 22.0 ],
                                    "text": "line 0. 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-61",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 2025.0, 636.0, 64.0, 22.0 ],
                                    "text": "pack 0. 80"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-141",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1830.0, 747.0, 150.0, 22.0 ],
                                    "text": "pattrforward parent::preset",
                                    "varname": "spread[3]"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-123",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 2040.0, 792.0, 150.0, 20.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-103",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 354.0, 1216.0, 150.0, 20.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-62",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "patching_rect": [ 1785.0, 255.0, 51.0, 22.0 ],
                                    "text": "line 0. 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-66",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1785.0, 221.0, 64.0, 22.0 ],
                                    "text": "pack 0. 80"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-36",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1703.9753784537315, 812.6125304698944, 65.0, 29.0 ],
                                    "text": "PAD 2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-37",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1703.9753784537315, 910.2869505882263, 81.0, 22.0 ],
                                    "text": "s load-default"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-40",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 1703.9753784537315, 884.7055548429489, 34.0, 22.0 ],
                                    "text": "sel 1"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-41",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1703.9753784537315, 857.5737714767456, 29.0, 22.0 ],
                                    "text": "r P01"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-185",
                                    "maxclass": "led",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1149.0, 376.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-183",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "patching_rect": [ 355.0, 242.0, 51.0, 22.0 ],
                                    "text": "line 0. 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-182",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 355.0, 208.0, 64.0, 22.0 ],
                                    "text": "pack 0. 80"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-176",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 412.0, 648.0, 33.0, 22.0 ],
                                    "text": "== 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-175",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 412.0, 674.0, 34.0, 22.0 ],
                                    "text": "sel 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-168",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 449.0, 729.0, 29.5, 22.0 ],
                                    "text": "f"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-164",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 459.0, 648.0, 74.0, 22.0 ],
                                    "text": "snapshot~ 5"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-159",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 459.0, 620.0, 94.0, 22.0 ],
                                    "text": "receive~ phasor"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-151",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "int", "int" ],
                                    "patching_rect": [ 459.0, 674.0, 61.0, 22.0 ],
                                    "text": "change 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-147",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 389.0, 891.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-144",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 449.0, 797.0, 40.0, 22.0 ],
                                    "text": "* 127."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-143",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 389.0, 858.0, 79.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "filename": "scalemode",
                                        "parameter_enable": 0
                                    },
                                    "text": "js scalemode"
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-142",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 449.0, 765.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-138",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 726.0, 1043.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-135",
                                    "maxclass": "toggle",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 965.0, 774.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-129",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1021.0, 729.0, 29.5, 22.0 ],
                                    "text": "- 1"
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-125",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 582.0, 108.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-121",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 2025.0, 509.0, 124.0, 29.0 ],
                                    "text": "TOUCH AVG"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-120",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1831.0, 509.0, 123.0, 29.0 ],
                                    "text": "ANY PRESS"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-117",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1538.0839030146599, 812.6125304698944, 124.0, 29.0 ],
                                    "text": "PAD 12 (CH)"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-116",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1342.735062777996, 814.1629180908203, 121.0, 29.0 ],
                                    "text": "PAD 11 (TO)"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-114",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1538.0, 508.0, 97.0, 29.0 ],
                                    "text": "SLIDER 2"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-112",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1342.0, 508.0, 104.0, 29.0 ],
                                    "text": "SWITCH 2"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-111",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 965.0, 508.0, 104.0, 29.0 ],
                                    "text": "SWITCH 1"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-107",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 355.0, 508.0, 97.0, 29.0 ],
                                    "text": "SLIDER 1"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-105",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1785.0, 36.0, 82.0, 29.0 ],
                                    "text": "KNOB 6"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-104",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1504.0, 36.0, 82.0, 29.0 ],
                                    "text": "KNOB 5"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-102",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1149.0, 36.0, 82.0, 29.0 ],
                                    "text": "KNOB 4"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-100",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 887.0, 36.0, 82.0, 29.0 ],
                                    "text": "KNOB 3"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-98",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 582.0, 36.0, 82.0, 29.0 ],
                                    "text": "KNOB 2"
                                }
                            },
                            {
                                "box": {
                                    "fontsize": 20.0,
                                    "id": "obj-95",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 354.0, 36.0, 82.0, 29.0 ],
                                    "text": "KNOB 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-91",
                                    "maxclass": "toggle",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1455.0, 638.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-90",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1455.0, 747.0, 40.0, 22.0 ],
                                    "text": "s fuzz"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-89",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1455.0, 602.0, 33.0, 22.0 ],
                                    "text": "== 2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-80",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1404.0, 747.0, 34.0, 22.0 ],
                                    "text": "s LED"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-76",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1342.0, 747.0, 45.0, 22.0 ],
                                    "text": "s mute"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-69",
                                    "maxclass": "toggle",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1342.0, 638.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-64",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1342.0, 602.0, 33.0, 22.0 ],
                                    "text": "== 1"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-63",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1342.0, 547.0, 37.0, 22.0 ],
                                    "text": "r S07"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-84",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 355.0, 1001.0, 32.0, 22.0 ],
                                    "text": "gate"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-83",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 355.0, 548.0, 260.0, 22.0 ],
                                    "restore": [ 0 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr mode @bindto parent::mode- @invisible 1",
                                    "varname": "mode[1]"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-82",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "patching_rect": [ 582.0, 407.0, 51.0, 22.0 ],
                                    "text": "line 0. 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-81",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 582.0, 374.0, 57.0, 22.0 ],
                                    "text": "pack f 80"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-79",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 1785.0, 299.0, 231.0, 22.0 ],
                                    "restore": [ 0.05533683289588802 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr buflength @bindto parent::buflength-",
                                    "varname": "buflength"
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-70",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1538.0, 580.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-68",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1507.0, 109.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-65",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1213.0, 108.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-55",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patcher": {
                                        "fileversion": 1,
                                        "appversion": {
                                            "major": 9,
                                            "minor": 2,
                                            "revision": 0,
                                            "architecture": "x64",
                                            "modernui": 1
                                        },
                                        "classnamespace": "box",
                                        "rect": [ 59.0, 110.0, 168.0, 259.0 ],
                                        "boxes": [
                                            {
                                                "box": {
                                                    "id": "obj-99",
                                                    "maxclass": "newobj",
                                                    "numinlets": 2,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "" ],
                                                    "patching_rect": [ 94.47514718770981, 154.1436612010002, 55.0, 22.0 ],
                                                    "text": "pipe 800"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "id": "obj-91",
                                                    "maxclass": "newobj",
                                                    "numinlets": 1,
                                                    "numoutlets": 3,
                                                    "outlettype": [ "int", "int", "stop" ],
                                                    "patching_rect": [ 50.00002219737621, 104.0, 58.0, 22.0 ],
                                                    "text": "t 0 1 stop"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-132",
                                                    "index": 1,
                                                    "maxclass": "inlet",
                                                    "numinlets": 0,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "bang" ],
                                                    "patching_rect": [ 50.00002219737621, 40.00001193177411, 30.0, 30.0 ]
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-133",
                                                    "index": 1,
                                                    "maxclass": "outlet",
                                                    "numinlets": 1,
                                                    "numoutlets": 0,
                                                    "patching_rect": [ 69.50002219737621, 204.97239536046982, 30.0, 30.0 ]
                                                }
                                            }
                                        ],
                                        "lines": [
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-91", 0 ],
                                                    "midpoints": [ 59.50002219737621, 72.0, 59.50002219737621, 72.0 ],
                                                    "source": [ "obj-132", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-133", 0 ],
                                                    "midpoints": [ 79.00002219737621, 129.0, 79.00002219737621, 129.0 ],
                                                    "source": [ "obj-91", 1 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-99", 0 ],
                                                    "midpoints": [ 98.50002219737621, 141.0, 103.97514718770981, 141.0 ],
                                                    "source": [ "obj-91", 2 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-99", 0 ],
                                                    "midpoints": [ 59.50002219737621, 141.0, 103.97514718770981, 141.0 ],
                                                    "source": [ "obj-91", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-133", 0 ],
                                                    "midpoints": [ 103.97514718770981, 189.0, 79.00002219737621, 189.0 ],
                                                    "source": [ "obj-99", 0 ]
                                                }
                                            }
                                        ]
                                    },
                                    "patching_rect": [ 1107.0, 868.0, 32.0, 22.0 ],
                                    "text": "p on"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-57",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1107.0, 899.0, 34.0, 22.0 ],
                                    "text": "s LED"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-59",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 1107.0, 838.0, 34.0, 22.0 ],
                                    "text": "sel 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-52",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patcher": {
                                        "fileversion": 1,
                                        "appversion": {
                                            "major": 9,
                                            "minor": 2,
                                            "revision": 0,
                                            "architecture": "x64",
                                            "modernui": 1
                                        },
                                        "classnamespace": "box",
                                        "rect": [ 59.0, 110.0, 168.0, 259.0 ],
                                        "boxes": [
                                            {
                                                "box": {
                                                    "id": "obj-99",
                                                    "maxclass": "newobj",
                                                    "numinlets": 2,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "" ],
                                                    "patching_rect": [ 94.47514718770981, 154.1436612010002, 55.0, 22.0 ],
                                                    "text": "pipe 800"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "id": "obj-91",
                                                    "maxclass": "newobj",
                                                    "numinlets": 1,
                                                    "numoutlets": 3,
                                                    "outlettype": [ "int", "int", "stop" ],
                                                    "patching_rect": [ 50.00002219737621, 104.0, 58.0, 22.0 ],
                                                    "text": "t 0 1 stop"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-132",
                                                    "index": 1,
                                                    "maxclass": "inlet",
                                                    "numinlets": 0,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "bang" ],
                                                    "patching_rect": [ 50.00002219737621, 40.00001193177411, 30.0, 30.0 ]
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-133",
                                                    "index": 1,
                                                    "maxclass": "outlet",
                                                    "numinlets": 1,
                                                    "numoutlets": 0,
                                                    "patching_rect": [ 69.50002219737621, 204.97239536046982, 30.0, 30.0 ]
                                                }
                                            }
                                        ],
                                        "lines": [
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-91", 0 ],
                                                    "midpoints": [ 59.50002219737621, 72.0, 59.50002219737621, 72.0 ],
                                                    "source": [ "obj-132", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-133", 0 ],
                                                    "midpoints": [ 79.00002219737621, 129.0, 79.00002219737621, 129.0 ],
                                                    "source": [ "obj-91", 1 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-99", 0 ],
                                                    "midpoints": [ 98.50002219737621, 141.0, 103.97514718770981, 141.0 ],
                                                    "source": [ "obj-91", 2 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-99", 0 ],
                                                    "midpoints": [ 59.50002219737621, 141.0, 103.97514718770981, 141.0 ],
                                                    "source": [ "obj-91", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-133", 0 ],
                                                    "midpoints": [ 103.97514718770981, 189.0, 79.00002219737621, 189.0 ],
                                                    "source": [ "obj-99", 0 ]
                                                }
                                            }
                                        ]
                                    },
                                    "patching_rect": [ 1149.0, 347.0, 32.0, 22.0 ],
                                    "text": "p on"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-50",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1149.0, 409.0, 34.0, 22.0 ],
                                    "text": "s LED"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-42",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 1149.0, 317.0, 34.0, 22.0 ],
                                    "text": "sel 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-32",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "int", "int" ],
                                    "patching_rect": [ 1149.0, 284.0, 48.0, 22.0 ],
                                    "text": "change"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-31",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1149.0, 253.0, 43.0, 22.0 ],
                                    "text": "== 0.5"
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-56",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 888.0, 108.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-53",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1785.0, 108.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-43",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 355.0, 108.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-23",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 1538.0, 621.7054359912872, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-30",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1538.0, 547.0, 29.0, 22.0 ],
                                    "text": "r S37"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-162",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1831.0, 675.0, 45.0, 22.0 ],
                                    "text": "store 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-21",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 1831.0, 642.0, 34.0, 22.0 ],
                                    "text": "sel 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-146",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "int", "int" ],
                                    "patching_rect": [ 1831.0, 608.0, 48.0, 22.0 ],
                                    "text": "change"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-22",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1831.0, 580.0, 33.0, 22.0 ],
                                    "text": "== 1"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-134",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1831.0, 549.0, 45.0, 22.0 ],
                                    "text": "r ACTIVE"
                                }
                            },
                            {
                                "box": {
                                    "format": 6,
                                    "id": "obj-149",
                                    "maxclass": "flonum",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 2025.0, 695.0, 50.0, 22.0 ],
                                    "varname": "number[14]"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-19",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 355.0, 276.0, 179.0, 22.0 ],
                                    "restore": [ 0.7952755905511811 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr start @bindto parent::start-",
                                    "varname": "start"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-8",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1785.0, 140.0, 79.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "filename": "scalemode",
                                        "parameter_enable": 0
                                    },
                                    "text": "js scalemode"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-136",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 888.0, 221.0, 41.0, 22.0 ],
                                    "text": "pow 4"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-18",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 646.0, 776.0, 41.0, 22.0 ],
                                    "text": "pow 2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-16",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "int", "int", "int" ],
                                    "patching_rect": [ 1006.0, 647.0, 40.0, 22.0 ],
                                    "text": "t i i i"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-14",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 713.0, 1095.0, 32.0, 22.0 ],
                                    "text": "gate"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-13",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 965.0, 697.0, 29.5, 22.0 ],
                                    "text": "!= 0"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-254",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1342.735062777996, 911.8373382091522, 151.0, 22.0 ],
                                    "text": "pattrforward parent::record",
                                    "varname": "spread[1]"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-256",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1342.735062777996, 859.1241590976715, 35.0, 22.0 ],
                                    "text": "r P10"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-252",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1507.0, 141.0, 79.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "filename": "scalemode",
                                        "parameter_enable": 0
                                    },
                                    "text": "js scalemode"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-251",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1213.0, 140.0, 79.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "filename": "scalemode",
                                        "parameter_enable": 0
                                    },
                                    "text": "js scalemode"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-250",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 888.0, 140.0, 79.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "filename": "scalemode",
                                        "parameter_enable": 0
                                    },
                                    "text": "js scalemode"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-246",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 646.0, 674.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-245",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 646.0, 620.0, 79.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "filename": "scalemode",
                                        "parameter_enable": 0
                                    },
                                    "text": "js scalemode"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-244",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 355.0, 140.0, 79.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "filename": "scalemode",
                                        "parameter_enable": 0
                                    },
                                    "text": "js scalemode"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-236",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1082.0, 547.0, 40.0, 22.0 ],
                                    "text": "r rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-237",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1082.0, 579.0, 104.0, 22.0 ],
                                    "text": "expr random(0\\, 3)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-234",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 705.0, 548.0, 40.0, 22.0 ],
                                    "text": "r rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-235",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 705.0, 580.0, 117.0, 22.0 ],
                                    "text": "expr random(0\\, 128)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-229",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 1507.0, 206.0, 41.0, 22.0 ],
                                    "text": "pow 3"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-227",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1567.0, 76.0, 40.0, 22.0 ],
                                    "text": "r rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-228",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1567.0, 108.0, 117.0, 22.0 ],
                                    "text": "expr random(0\\, 128)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-226",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1213.0, 283.0, 63.0, 22.0 ],
                                    "text": "clip 0.01 2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-210",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1272.0, 76.0, 40.0, 22.0 ],
                                    "text": "r rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-211",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1272.0, 108.0, 117.0, 22.0 ],
                                    "text": "expr random(0\\, 128)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-207",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 948.0, 76.0, 40.0, 22.0 ],
                                    "text": "r rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-209",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 948.0, 108.0, 117.0, 22.0 ],
                                    "text": "expr random(0\\, 128)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-204",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1845.0, 76.0, 40.0, 22.0 ],
                                    "text": "r rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-206",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1845.0, 105.0, 117.0, 22.0 ],
                                    "text": "expr random(0\\, 128)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-203",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1538.0839030146599, 910.2869505882263, 42.0, 22.0 ],
                                    "text": "s rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-202",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 1538.0839030146599, 884.7055548429489, 34.0, 22.0 ],
                                    "text": "sel 1"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-199",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1538.0839030146599, 857.5737714767456, 36.0, 22.0 ],
                                    "text": "r P11"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-198",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 415.0, 76.0, 40.0, 22.0 ],
                                    "text": "r rand"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-196",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 415.0, 108.0, 117.0, 22.0 ],
                                    "text": "expr random(0\\, 128)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-178",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 693.0, 870.0, 29.5, 22.0 ],
                                    "text": "+ 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-174",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 646.0, 870.0, 29.5, 22.0 ],
                                    "text": "i"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-173",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "int" ],
                                    "patching_rect": [ 1021.0, 824.0, 29.5, 22.0 ],
                                    "text": "t b i"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-172",
                                    "maxclass": "number",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 713.0, 1129.0, 50.0, 22.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-170",
                                    "maxclass": "toggle",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 1021.0, 774.0, 24.0, 24.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-154",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 726.0, 1006.0, 29.5, 22.0 ],
                                    "text": "i"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-126",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "" ],
                                    "patching_rect": [ 693.0, 924.0, 52.0, 22.0 ],
                                    "text": "gate 2 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-97",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "bang", "" ],
                                    "patching_rect": [ 1006.0, 697.0, 34.0, 22.0 ],
                                    "text": "sel 0"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-94",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 693.0, 955.0, 29.5, 22.0 ],
                                    "text": "* -1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-155",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 1107.0, 699.0, 33.0, 22.0 ],
                                    "text": "== 0"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-177",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 582.0, 441.0, 228.0, 22.0 ],
                                    "restore": [ 128.9654652800119 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr grainsize @bindto parent::grainsize-",
                                    "varname": "grainsize"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-131",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 582.0, 260.0, 29.5, 22.0 ],
                                    "text": "/ 2."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-127",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 582.0, 232.0, 41.0, 22.0 ],
                                    "text": "pow 3"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-74",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 582.0, 312.0, 29.5, 22.0 ],
                                    "text": "* 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-75",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 623.0, 260.0, 49.0, 22.0 ],
                                    "text": "r length"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-12",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 1538.0, 747.0, 246.0, 22.0 ],
                                    "restore": [ 0.4692789385578771 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr gain @bindto parent::gain- @invisible 1",
                                    "varname": "gain"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-4",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 1538.0, 666.0, 41.0, 22.0 ],
                                    "text": "pow 2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-54",
                                    "maxclass": "newobj",
                                    "numinlets": 6,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 2025.0, 607.0, 83.0, 22.0 ],
                                    "text": "scale 0. 1. 1 8"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-51",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 2025.0, 578.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-49",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 2025.0, 549.0, 45.0, 22.0 ],
                                    "text": "r TOUCH"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-48",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 1107.0, 775.0, 192.0, 22.0 ],
                                    "restore": [ 0 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr mode @bindto parent::mode-",
                                    "varname": "mode"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-47",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1006.0, 547.0, 37.0, 22.0 ],
                                    "text": "r S09"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-44",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "int", "int" ],
                                    "patching_rect": [ 1213.0, 313.0, 61.0, 22.0 ],
                                    "text": "change 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-45",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1213.0, 206.0, 254.0, 22.0 ],
                                    "text": "if $f1 >= 0.48 && $f1 <= 0.52 then 0.5 else $f1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-46",
                                    "maxclass": "newobj",
                                    "numinlets": 6,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1213.0, 253.0, 87.0, 22.0 ],
                                    "text": "scale 0. 1. 0. 2"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-34",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "int", "int" ],
                                    "patching_rect": [ 582.0, 201.0, 61.0, 22.0 ],
                                    "text": "change 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-35",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 582.0, 169.0, 254.0, 22.0 ],
                                    "text": "if $f1 >= 0.45 && $f1 <= 0.55 then 0.5 else $f1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-38",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 582.0, 140.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-39",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 582.0, 76.0, 29.0, 22.0 ],
                                    "text": "r S31"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-33",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 355.0, 1172.0, 268.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "initial": [ 0.5 ],
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr spread @bindto parent::spread- @initial 0.5",
                                    "varname": "spread"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-24",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 1507.0, 246.0, 231.0, 22.0 ],
                                    "restore": [ 0.41856186074576873 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr randpitch @bindto parent::randpitch-",
                                    "varname": "randpitch"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-25",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 1507.0, 176.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-26",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1507.0, 76.0, 29.0, 22.0 ],
                                    "text": "r S34"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-27",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 1213.0, 409.0, 183.0, 22.0 ],
                                    "restore": [ 0.7716535433070866 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr pitch @bindto parent::pitch-",
                                    "varname": "pitch"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-28",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 1213.0, 176.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-29",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1213.0, 76.0, 29.0, 22.0 ],
                                    "text": "r S33"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-17",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "int", "int" ],
                                    "patching_rect": [ 646.0, 745.0, 61.0, 22.0 ],
                                    "text": "change 0."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-15",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 646.0, 714.0, 254.0, 22.0 ],
                                    "text": "if $f1 >= 0.45 && $f1 <= 0.55 then 0.5 else $f1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-10",
                                    "maxclass": "newobj",
                                    "numinlets": 6,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 646.0, 809.0, 97.0, 22.0 ],
                                    "text": "scale 0. 1. 0 400"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-7",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 713.0, 1172.0, 264.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "initial": [ 100 ],
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr speed @bindto parent::speed- @initial 100",
                                    "varname": "speed"
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-9",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 646.0, 548.0, 29.0, 22.0 ],
                                    "text": "r S36"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-5",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 355.0, 177.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-6",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 355.0, 76.0, 29.0, 22.0 ],
                                    "text": "r S30"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-1",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 888.0, 253.0, 219.0, 22.0 ],
                                    "restore": [ 0.00014767169468618804 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr randpos @bindto parent::randpos-",
                                    "varname": "randpos"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-2",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 888.0, 176.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-3",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 888.0, 76.0, 29.0, 22.0 ],
                                    "text": "r S32"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-157",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 355.0, 1034.0, 171.0, 22.0 ],
                                    "restore": [ 0.05745661021226802 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr pos @bindto parent::pos-",
                                    "varname": "pos"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-71",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 1785.0, 177.0, 39.0, 22.0 ],
                                    "text": "/ 127."
                                }
                            },
                            {
                                "box": {
                                    "color": [ 0.8274509803921568, 0.6862745098039216, 0.21568627450980393, 1.0 ],
                                    "fontsize": 12.0,
                                    "id": "obj-67",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 1785.0, 76.0, 29.0, 22.0 ],
                                    "text": "r S35"
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-99",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1763.0, 21.0, 271.0, 460.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-96",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1493.0, 21.0, 265.0, 460.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-93",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1132.0, 21.0, 356.0, 460.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-88",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 871.0, 21.0, 251.0, 270.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-87",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 561.0, 21.0, 304.0, 460.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.611764705882353, 0.39215686274509803, 0.13725490196078433, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-119",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1810.0, 490.0, 190.0, 295.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.6980392156862745, 0.7215686274509804, 0.17647058823529413, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-122",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 2005.0, 490.0, 158.0, 295.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.09019607843137255, 0.16470588235294117, 0.3686274509803922, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-115",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1323.3552175164223, 790.9071037769318, 191.28956496715546, 167.09289622306824 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.09019607843137255, 0.16470588235294117, 0.3686274509803922, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-118",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1517.9288639426231, 790.9071037769318, 160.0, 166.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.09019607843137255, 0.16470588235294117, 0.3686274509803922, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-58",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1683.8203393816948, 790.9071037769318, 160.0, 166.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.08235294117647059, 0.25882352941176473, 0.13333333333333333, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-113",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1520.1550623178482, 489.92248821258545, 285.2713222503662, 295.07751178741455 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.3686274509803922, 0.09019607843137255, 0.09019607843137255, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-110",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 1323.0, 490.0, 192.0, 295.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.3686274509803922, 0.09019607843137255, 0.09019607843137255, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-109",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 938.0, 490.0, 379.0, 468.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.08235294117647059, 0.25882352941176473, 0.13333333333333333, 0.5 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-108",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 331.0, 490.0, 599.0, 724.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            },
                            {
                                "box": {
                                    "angle": 270.0,
                                    "background": 1,
                                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                                    "border": 1,
                                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                                    "id": "obj-92",
                                    "maxclass": "panel",
                                    "mode": 0,
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 331.0, 21.0, 216.0, 300.0 ],
                                    "proportion": 0.5,
                                    "rounded": 0
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-174", 0 ],
                                    "midpoints": [ 655.5, 834.0, 655.5, 834.0 ],
                                    "source": [ "obj-10", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-130", 0 ],
                                    "midpoints": [ 590.5, 924.0, 589.5, 924.0 ],
                                    "source": [ "obj-124", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-38", 0 ],
                                    "midpoints": [ 591.5, 132.0, 591.5, 132.0 ],
                                    "source": [ "obj-125", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-154", 0 ],
                                    "midpoints": [ 735.5, 948.0, 735.5, 948.0 ],
                                    "source": [ "obj-126", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-94", 0 ],
                                    "midpoints": [ 702.5, 948.0, 702.5, 948.0 ],
                                    "source": [ "obj-126", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-131", 0 ],
                                    "midpoints": [ 591.5, 255.0, 591.5, 255.0 ],
                                    "source": [ "obj-127", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-124", 0 ],
                                    "midpoints": [ 590.5, 894.0, 590.5, 894.0 ],
                                    "source": [ "obj-128", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-170", 0 ],
                                    "midpoints": [ 1030.5, 753.0, 1030.5, 753.0 ],
                                    "source": [ "obj-129", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-135", 0 ],
                                    "midpoints": [ 974.5, 720.0, 974.5, 720.0 ],
                                    "source": [ "obj-13", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-74", 0 ],
                                    "midpoints": [ 591.5, 285.0, 591.5, 285.0 ],
                                    "source": [ "obj-131", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-128", 0 ],
                                    "midpoints": [ 590.5, 864.0, 590.5, 864.0 ],
                                    "source": [ "obj-132", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-132", 0 ],
                                    "midpoints": [ 590.5, 831.0, 590.5, 831.0 ],
                                    "source": [ "obj-133", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-22", 0 ],
                                    "midpoints": [ 1840.5, 573.0, 1840.5, 573.0 ],
                                    "source": [ "obj-134", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 0 ],
                                    "midpoints": [ 974.5, 1080.0, 722.5, 1080.0 ],
                                    "source": [ "obj-135", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-1", 0 ],
                                    "midpoints": [ 897.5, 246.0, 897.5, 246.0 ],
                                    "source": [ "obj-136", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-133", 0 ],
                                    "midpoints": [ 590.5, 801.0, 590.5, 801.0 ],
                                    "source": [ "obj-137", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 1 ],
                                    "midpoints": [ 735.5, 1068.0, 735.5, 1068.0 ],
                                    "source": [ "obj-138", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-172", 0 ],
                                    "midpoints": [ 722.5, 1119.0, 722.5, 1119.0 ],
                                    "source": [ "obj-14", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-139", 0 ],
                                    "source": [ "obj-140", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-144", 0 ],
                                    "midpoints": [ 458.5, 789.0, 458.5, 789.0 ],
                                    "source": [ "obj-142", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-147", 0 ],
                                    "midpoints": [ 398.5, 882.0, 398.5, 882.0 ],
                                    "source": [ "obj-143", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-143", 1 ],
                                    "midpoints": [ 458.5, 822.0, 458.5, 822.0 ],
                                    "source": [ "obj-144", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-21", 0 ],
                                    "midpoints": [ 1840.5, 633.0, 1840.5, 633.0 ],
                                    "source": [ "obj-146", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-84", 1 ],
                                    "midpoints": [ 398.5, 987.0, 377.5, 987.0 ],
                                    "source": [ "obj-147", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-141", 0 ],
                                    "midpoints": [ 2034.5, 732.0, 1839.5, 732.0 ],
                                    "source": [ "obj-149", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-17", 0 ],
                                    "midpoints": [ 655.5, 738.0, 655.5, 738.0 ],
                                    "source": [ "obj-15", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-168", 1 ],
                                    "midpoints": [ 468.5, 699.0, 469.0, 699.0 ],
                                    "source": [ "obj-151", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-138", 0 ],
                                    "midpoints": [ 735.5, 1029.0, 735.5, 1029.0 ],
                                    "source": [ "obj-154", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-48", 0 ],
                                    "midpoints": [ 1116.5, 723.0, 1116.5, 723.0 ],
                                    "order": 1,
                                    "source": [ "obj-155", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-59", 0 ],
                                    "midpoints": [ 1116.5, 762.0, 1092.0, 762.0, 1092.0, 825.0, 1116.5, 825.0 ],
                                    "order": 0,
                                    "source": [ "obj-155", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-164", 0 ],
                                    "midpoints": [ 468.5, 645.0, 468.5, 645.0 ],
                                    "source": [ "obj-159", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-13", 0 ],
                                    "midpoints": [ 1026.0, 684.0, 974.5, 684.0 ],
                                    "source": [ "obj-16", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-155", 0 ],
                                    "midpoints": [ 1036.5, 684.0, 1116.5, 684.0 ],
                                    "source": [ "obj-16", 2 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-97", 0 ],
                                    "midpoints": [ 1015.5, 672.0, 1015.5, 672.0 ],
                                    "source": [ "obj-16", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-141", 0 ],
                                    "midpoints": [ 1840.5, 699.0, 1839.5, 699.0 ],
                                    "source": [ "obj-162", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-151", 0 ],
                                    "midpoints": [ 468.5, 672.0, 468.5, 672.0 ],
                                    "source": [ "obj-164", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-142", 0 ],
                                    "midpoints": [ 458.5, 753.0, 458.5, 753.0 ],
                                    "source": [ "obj-168", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-137", 0 ],
                                    "midpoints": [ 655.5, 768.0, 590.5, 768.0 ],
                                    "order": 1,
                                    "source": [ "obj-17", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-18", 0 ],
                                    "midpoints": [ 655.5, 768.0, 655.5, 768.0 ],
                                    "order": 0,
                                    "source": [ "obj-17", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-173", 0 ],
                                    "midpoints": [ 1030.5, 801.0, 1030.5, 801.0 ],
                                    "source": [ "obj-170", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-7", 0 ],
                                    "midpoints": [ 722.5, 1152.0, 722.5, 1152.0 ],
                                    "source": [ "obj-172", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-174", 0 ],
                                    "midpoints": [ 1030.5, 849.0, 655.5, 849.0 ],
                                    "source": [ "obj-173", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-178", 0 ],
                                    "midpoints": [ 1041.0, 867.0, 702.5, 867.0 ],
                                    "source": [ "obj-173", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-126", 1 ],
                                    "midpoints": [ 655.5, 909.0, 735.5, 909.0 ],
                                    "source": [ "obj-174", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-168", 0 ],
                                    "midpoints": [ 421.5, 714.0, 458.5, 714.0 ],
                                    "source": [ "obj-175", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-175", 0 ],
                                    "midpoints": [ 421.5, 672.0, 421.5, 672.0 ],
                                    "source": [ "obj-176", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-126", 0 ],
                                    "midpoints": [ 702.5, 894.0, 702.5, 894.0 ],
                                    "source": [ "obj-178", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-10", 0 ],
                                    "midpoints": [ 655.5, 801.0, 655.5, 801.0 ],
                                    "source": [ "obj-18", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-183", 0 ],
                                    "midpoints": [ 364.5, 231.0, 364.5, 231.0 ],
                                    "source": [ "obj-182", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-19", 0 ],
                                    "midpoints": [ 364.5, 267.0, 364.5, 267.0 ],
                                    "source": [ "obj-183", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-50", 0 ],
                                    "midpoints": [ 1158.5, 402.0, 1158.5, 402.0 ],
                                    "source": [ "obj-185", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-244", 1 ],
                                    "midpoints": [ 424.5, 132.0, 424.5, 132.0 ],
                                    "source": [ "obj-196", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-196", 0 ],
                                    "midpoints": [ 424.5, 99.0, 424.5, 99.0 ],
                                    "source": [ "obj-198", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-202", 0 ],
                                    "midpoints": [ 1547.5839030146599, 882.0, 1547.5839030146599, 882.0 ],
                                    "source": [ "obj-199", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-136", 0 ],
                                    "midpoints": [ 897.5, 201.0, 897.5, 201.0 ],
                                    "source": [ "obj-2", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-27", 0 ],
                                    "midpoints": [ 1222.5, 405.0, 1222.5, 405.0 ],
                                    "source": [ "obj-20", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-203", 0 ],
                                    "midpoints": [ 1547.5839030146599, 909.0, 1547.5839030146599, 909.0 ],
                                    "source": [ "obj-202", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-206", 0 ],
                                    "midpoints": [ 1854.5, 99.0, 1854.5, 99.0 ],
                                    "source": [ "obj-204", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-8", 1 ],
                                    "midpoints": [ 1854.5, 129.0, 1854.5, 129.0 ],
                                    "source": [ "obj-206", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-209", 0 ],
                                    "midpoints": [ 957.5, 99.0, 957.5, 99.0 ],
                                    "source": [ "obj-207", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-250", 1 ],
                                    "midpoints": [ 957.5, 132.0, 957.5, 132.0 ],
                                    "source": [ "obj-209", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-162", 0 ],
                                    "midpoints": [ 1840.5, 666.0, 1840.5, 666.0 ],
                                    "source": [ "obj-21", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-211", 0 ],
                                    "midpoints": [ 1281.5, 99.0, 1281.5, 99.0 ],
                                    "source": [ "obj-210", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-251", 1 ],
                                    "midpoints": [ 1281.5, 132.0, 1282.5, 132.0 ],
                                    "source": [ "obj-211", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-146", 0 ],
                                    "midpoints": [ 1840.5, 603.0, 1840.5, 603.0 ],
                                    "source": [ "obj-22", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-44", 0 ],
                                    "midpoints": [ 1222.5, 306.0, 1222.5, 306.0 ],
                                    "source": [ "obj-226", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-228", 0 ],
                                    "midpoints": [ 1576.5, 99.0, 1576.5, 99.0 ],
                                    "source": [ "obj-227", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-252", 1 ],
                                    "midpoints": [ 1576.5, 132.0, 1576.5, 132.0 ],
                                    "source": [ "obj-228", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-24", 0 ],
                                    "midpoints": [ 1516.5, 231.0, 1516.5, 231.0 ],
                                    "source": [ "obj-229", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-4", 0 ],
                                    "midpoints": [ 1547.5, 645.0, 1547.5, 645.0 ],
                                    "source": [ "obj-23", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-235", 0 ],
                                    "midpoints": [ 714.5, 573.0, 714.5, 573.0 ],
                                    "source": [ "obj-234", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-245", 1 ],
                                    "midpoints": [ 714.5, 603.0, 715.5, 603.0 ],
                                    "source": [ "obj-235", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-237", 0 ],
                                    "midpoints": [ 1091.5, 570.0, 1091.5, 570.0 ],
                                    "source": [ "obj-236", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-5", 0 ],
                                    "midpoints": [ 364.5, 165.0, 364.5, 165.0 ],
                                    "source": [ "obj-244", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-143", 0 ],
                                    "midpoints": [ 655.5, 660.0, 543.0, 660.0, 543.0, 843.0, 398.5, 843.0 ],
                                    "order": 1,
                                    "source": [ "obj-245", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-246", 0 ],
                                    "midpoints": [ 655.5, 645.0, 655.5, 645.0 ],
                                    "order": 0,
                                    "source": [ "obj-245", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-15", 0 ],
                                    "midpoints": [ 655.5, 699.0, 655.5, 699.0 ],
                                    "source": [ "obj-246", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-229", 0 ],
                                    "midpoints": [ 1516.5, 201.0, 1516.5, 201.0 ],
                                    "source": [ "obj-25", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-2", 0 ],
                                    "midpoints": [ 897.5, 165.0, 897.5, 165.0 ],
                                    "source": [ "obj-250", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-28", 0 ],
                                    "midpoints": [ 1222.5, 165.0, 1222.5, 165.0 ],
                                    "source": [ "obj-251", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-25", 0 ],
                                    "midpoints": [ 1516.5, 165.0, 1516.5, 165.0 ],
                                    "source": [ "obj-252", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-68", 0 ],
                                    "midpoints": [ 1516.5, 99.0, 1516.5, 99.0 ],
                                    "source": [ "obj-26", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-45", 0 ],
                                    "midpoints": [ 1222.5, 201.0, 1222.5, 201.0 ],
                                    "source": [ "obj-28", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-65", 0 ],
                                    "midpoints": [ 1222.5, 99.0, 1222.5, 99.0 ],
                                    "source": [ "obj-29", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-56", 0 ],
                                    "midpoints": [ 897.5, 99.0, 897.5, 99.0 ],
                                    "source": [ "obj-3", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-70", 0 ],
                                    "midpoints": [ 1547.5, 570.0, 1547.5, 570.0 ],
                                    "source": [ "obj-30", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-32", 0 ],
                                    "midpoints": [ 1158.5, 276.0, 1158.5, 276.0 ],
                                    "source": [ "obj-31", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-42", 0 ],
                                    "midpoints": [ 1158.5, 309.0, 1158.5, 309.0 ],
                                    "source": [ "obj-32", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-127", 0 ],
                                    "midpoints": [ 591.5, 225.0, 591.5, 225.0 ],
                                    "source": [ "obj-34", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-34", 0 ],
                                    "midpoints": [ 591.5, 192.0, 591.5, 192.0 ],
                                    "source": [ "obj-35", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-35", 0 ],
                                    "midpoints": [ 591.5, 165.0, 591.5, 165.0 ],
                                    "source": [ "obj-38", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-125", 0 ],
                                    "midpoints": [ 591.5, 99.0, 591.5, 99.0 ],
                                    "source": [ "obj-39", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-12", 0 ],
                                    "midpoints": [ 1547.5, 690.0, 1547.5, 690.0 ],
                                    "source": [ "obj-4", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-37", 0 ],
                                    "midpoints": [ 1713.4753784537315, 909.0, 1713.4753784537315, 909.0 ],
                                    "source": [ "obj-40", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-40", 0 ],
                                    "midpoints": [ 1713.4753784537315, 882.0, 1713.4753784537315, 882.0 ],
                                    "source": [ "obj-41", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-52", 0 ],
                                    "midpoints": [ 1158.5, 342.0, 1158.5, 342.0 ],
                                    "source": [ "obj-42", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-244", 0 ],
                                    "midpoints": [ 364.5, 132.0, 364.5, 132.0 ],
                                    "source": [ "obj-43", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-72", 0 ],
                                    "midpoints": [ 1222.5, 336.0, 1222.5, 336.0 ],
                                    "source": [ "obj-44", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-31", 0 ],
                                    "midpoints": [ 1222.5, 240.0, 1158.5, 240.0 ],
                                    "order": 1,
                                    "source": [ "obj-45", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-46", 0 ],
                                    "midpoints": [ 1222.5, 231.0, 1222.5, 231.0 ],
                                    "order": 0,
                                    "source": [ "obj-45", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-226", 0 ],
                                    "midpoints": [ 1222.5, 276.0, 1222.5, 276.0 ],
                                    "source": [ "obj-46", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-16", 0 ],
                                    "midpoints": [ 1015.5, 570.0, 1015.5, 570.0 ],
                                    "source": [ "obj-47", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-51", 0 ],
                                    "midpoints": [ 2034.5, 573.0, 2034.5, 573.0 ],
                                    "source": [ "obj-49", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-182", 0 ],
                                    "midpoints": [ 364.5, 201.0, 364.5, 201.0 ],
                                    "source": [ "obj-5", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-54", 0 ],
                                    "midpoints": [ 2034.5, 603.0, 2034.5, 603.0 ],
                                    "source": [ "obj-51", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-185", 0 ],
                                    "midpoints": [ 1158.5, 372.0, 1158.5, 372.0 ],
                                    "source": [ "obj-52", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-8", 0 ],
                                    "midpoints": [ 1794.5, 132.0, 1794.5, 132.0 ],
                                    "source": [ "obj-53", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-61", 0 ],
                                    "midpoints": [ 2034.5, 630.0, 2034.5, 630.0 ],
                                    "source": [ "obj-54", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-57", 0 ],
                                    "midpoints": [ 1116.5, 891.0, 1116.5, 891.0 ],
                                    "source": [ "obj-55", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-250", 0 ],
                                    "midpoints": [ 897.5, 132.0, 897.5, 132.0 ],
                                    "source": [ "obj-56", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-55", 0 ],
                                    "midpoints": [ 1116.5, 861.0, 1116.5, 861.0 ],
                                    "source": [ "obj-59", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-43", 0 ],
                                    "midpoints": [ 364.5, 99.0, 364.5, 99.0 ],
                                    "source": [ "obj-6", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-149", 0 ],
                                    "midpoints": [ 2034.5, 690.0, 2034.5, 690.0 ],
                                    "source": [ "obj-60", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-60", 0 ],
                                    "midpoints": [ 2034.5, 660.0, 2034.5, 660.0 ],
                                    "source": [ "obj-61", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-79", 0 ],
                                    "midpoints": [ 1794.5, 279.0, 1794.5, 279.0 ],
                                    "source": [ "obj-62", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-64", 0 ],
                                    "midpoints": [ 1351.5, 570.0, 1351.5, 570.0 ],
                                    "order": 1,
                                    "source": [ "obj-63", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-89", 0 ],
                                    "midpoints": [ 1351.5, 588.0, 1464.5, 588.0 ],
                                    "order": 0,
                                    "source": [ "obj-63", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-69", 0 ],
                                    "midpoints": [ 1351.5, 627.0, 1351.5, 627.0 ],
                                    "source": [ "obj-64", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-251", 0 ],
                                    "midpoints": [ 1222.5, 132.0, 1222.5, 132.0 ],
                                    "source": [ "obj-65", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-62", 0 ],
                                    "midpoints": [ 1794.5, 246.0, 1794.5, 246.0 ],
                                    "source": [ "obj-66", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-53", 0 ],
                                    "midpoints": [ 1794.5, 99.0, 1794.5, 99.0 ],
                                    "source": [ "obj-67", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-252", 0 ],
                                    "midpoints": [ 1516.5, 132.0, 1516.5, 132.0 ],
                                    "source": [ "obj-68", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-76", 0 ],
                                    "midpoints": [ 1351.5, 663.0, 1351.5, 663.0 ],
                                    "order": 1,
                                    "source": [ "obj-69", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-80", 0 ],
                                    "midpoints": [ 1351.5, 732.0, 1413.5, 732.0 ],
                                    "order": 0,
                                    "source": [ "obj-69", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-23", 0 ],
                                    "midpoints": [ 1547.5, 603.0, 1547.5, 603.0 ],
                                    "source": [ "obj-70", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-66", 0 ],
                                    "midpoints": [ 1794.5, 201.0, 1794.5, 201.0 ],
                                    "source": [ "obj-71", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-20", 0 ],
                                    "midpoints": [ 1222.5, 372.0, 1222.5, 372.0 ],
                                    "source": [ "obj-72", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-81", 0 ],
                                    "midpoints": [ 591.5, 336.0, 591.5, 336.0 ],
                                    "source": [ "obj-74", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-74", 1 ],
                                    "midpoints": [ 632.5, 297.0, 602.0, 297.0 ],
                                    "source": [ "obj-75", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-86", 0 ],
                                    "midpoints": [ 1884.69382750988, 879.2558267116547, 1884.69382750988, 879.2558267116547 ],
                                    "source": [ "obj-77", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-140", 0 ],
                                    "source": [ "obj-78", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-71", 0 ],
                                    "midpoints": [ 1794.5, 165.0, 1794.5, 165.0 ],
                                    "source": [ "obj-8", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-82", 0 ],
                                    "midpoints": [ 591.5, 399.0, 591.5, 399.0 ],
                                    "source": [ "obj-81", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-177", 0 ],
                                    "midpoints": [ 591.5, 432.0, 591.5, 432.0 ],
                                    "source": [ "obj-82", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-176", 0 ],
                                    "midpoints": [ 364.5, 633.0, 421.5, 633.0 ],
                                    "order": 0,
                                    "source": [ "obj-83", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-84", 0 ],
                                    "midpoints": [ 364.5, 573.0, 364.5, 573.0 ],
                                    "order": 1,
                                    "source": [ "obj-83", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-157", 0 ],
                                    "midpoints": [ 364.5, 1026.0, 364.5, 1026.0 ],
                                    "source": [ "obj-84", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-77", 0 ],
                                    "midpoints": [ 1884.69382750988, 846.2558267116547, 1884.69382750988, 846.2558267116547 ],
                                    "source": [ "obj-85", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-78", 0 ],
                                    "midpoints": [ 1884.69382750988, 921.2558267116547, 1884.69382750988, 921.2558267116547 ],
                                    "source": [ "obj-86", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-91", 0 ],
                                    "midpoints": [ 1464.5, 627.0, 1464.5, 627.0 ],
                                    "source": [ "obj-89", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-245", 0 ],
                                    "midpoints": [ 655.5, 573.0, 655.5, 573.0 ],
                                    "source": [ "obj-9", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-90", 0 ],
                                    "midpoints": [ 1464.5, 663.0, 1464.5, 663.0 ],
                                    "source": [ "obj-91", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-154", 0 ],
                                    "midpoints": [ 702.5, 993.0, 735.5, 993.0 ],
                                    "source": [ "obj-94", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-129", 0 ],
                                    "midpoints": [ 1030.5, 720.0, 1030.5, 720.0 ],
                                    "source": [ "obj-97", 1 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 908.0, 116.0, 71.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 551.1451074779034, 391.0, 71.0, 22.0 ],
                    "text": "p mappings",
                    "varname": "mappings"
                }
            },
            {
                "box": {
                    "id": "obj-152",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "signal", "signal" ],
                    "patching_rect": [ 2824.0, 63.0, 35.0, 22.0 ],
                    "text": "adc~"
                }
            },
            {
                "box": {
                    "id": "obj-60",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 469.0, 437.0, 41.0, 22.0 ],
                    "text": "set $1"
                }
            },
            {
                "box": {
                    "id": "obj-9",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 469.0, 378.0, 29.5, 22.0 ],
                    "text": "/ 0."
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-35",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 671.0, 219.0, 50.0, 22.0 ],
                    "varname": "randpo"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-80",
                    "maxclass": "flonum",
                    "maximum": 1.0,
                    "minimum": 0.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 671.008363366127, 92.85713732242584, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 16.0, 322.0, 50.0, 22.0 ],
                    "varname": "randpos-"
                }
            },
            {
                "box": {
                    "id": "obj-70",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 671.008363366127, 139.0756219625473, 29.5, 22.0 ],
                    "text": "* 0."
                }
            },
            {
                "box": {
                    "id": "obj-68",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 559.0, 329.0, 49.0, 22.0 ],
                    "text": "r length"
                }
            },
            {
                "box": {
                    "id": "obj-46",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2349.0, 94.0, 69.0, 22.0 ],
                    "text": "rainstick.aif"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-156",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2434.0, 62.0, 79.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 362.0, 86.0, 79.0, 22.0 ],
                    "text": "load audiofile"
                }
            },
            {
                "box": {
                    "id": "obj-154",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "patching_rect": [ 2434.0, 94.0, 67.0, 22.0 ],
                    "text": "opendialog"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-1",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2151.0, 389.0, 35.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 445.55245208740234, 86.0, 35.0, 22.0 ],
                    "text": "clear"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-72",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 929.0, 22.0, 42.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 214.0, 14.0, 42.0, 20.0 ],
                    "text": "preset"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-126",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "", "int", "int" ],
                    "patching_rect": [ 257.0, 258.5, 48.0, 22.0 ],
                    "text": "change"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-124",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "patching_rect": [ 257.0, 232.0, 33.0, 22.0 ],
                    "text": "== 0"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-123",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 257.0, 285.0, 41.0, 22.0 ],
                    "text": "set $1"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-120",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 2,
                    "outlettype": [ "bang", "" ],
                    "patching_rect": [ 216.0, 232.0, 34.0, 22.0 ],
                    "text": "sel 0"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-119",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 216.0, 133.0, 29.5, 22.0 ],
                    "text": "0"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-116",
                    "maxclass": "newobj",
                    "numinlets": 3,
                    "numoutlets": 3,
                    "outlettype": [ "bang", "bang", "" ],
                    "patching_rect": [ 191.0, 94.0, 44.0, 22.0 ],
                    "text": "sel 0 1"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-111",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "patching_rect": [ 215.0, 285.0, 35.0, 22.0 ],
                    "text": "i 100"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-107",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 1348.0, 240.0, 33.0, 22.0 ],
                    "text": "* 0.5"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-78",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2232.0, 34.0, 90.0, 20.0 ],
                    "text": "import audiofile"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-77",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 2,
                            "revision": 0,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 249.0, 234.0, 791.0, 425.0 ],
                        "boxes": [
                            {
                                "box": {
                                    "id": "obj-1",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 498.0, 70.0, 41.0, 22.0 ],
                                    "text": "unjoin"
                                }
                            },
                            {
                                "box": {
                                    "comment": "",
                                    "id": "obj-8",
                                    "index": 1,
                                    "maxclass": "inlet",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "int" ],
                                    "patching_rect": [ 427.0, 34.0, 30.0, 30.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-157",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 498.0, 179.0, 239.0, 22.0 ],
                                    "restore": [ 0.05745661021226802 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr pos @bindto parent::pos- @invisible 1",
                                    "varname": "pos"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-44",
                                    "maxclass": "newobj",
                                    "numinlets": 0,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 498.0, 34.0, 52.0, 22.0 ],
                                    "text": "r mouse"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-54",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "" ],
                                    "patching_rect": [ 498.0, 140.0, 47.0, 22.0 ],
                                    "text": "zl.nth 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-167",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 334.0, 167.0, 65.0, 22.0 ],
                                    "text": "pvar pitch-"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-147",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 471.0, 300.0, 251.0, 22.0 ],
                                    "restore": [ 0.7716535433070866 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr pitch @bindto parent::pitch- @invisible 1",
                                    "varname": "pitch"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-79",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patcher": {
                                        "fileversion": 1,
                                        "appversion": {
                                            "major": 9,
                                            "minor": 2,
                                            "revision": 0,
                                            "architecture": "x64",
                                            "modernui": 1
                                        },
                                        "classnamespace": "box",
                                        "rect": [ 59.0, 110.0, 640.0, 480.0 ],
                                        "default_fontsize": 10.0,
                                        "default_fontname": "Osaka",
                                        "boxes": [
                                            {
                                                "box": {
                                                    "fontname": "Osaka",
                                                    "fontsize": 12.0,
                                                    "id": "obj-70",
                                                    "maxclass": "newobj",
                                                    "numinlets": 2,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "int" ],
                                                    "patching_rect": [ 125.0, 100.0, 34.0, 23.0 ],
                                                    "text": "!= 1"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "fontname": "Osaka",
                                                    "fontsize": 12.0,
                                                    "id": "obj-4",
                                                    "maxclass": "message",
                                                    "numinlets": 2,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "" ],
                                                    "patching_rect": [ 50.0, 127.0, 71.0, 23.0 ],
                                                    "text": "triangle $1"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "fontname": "Osaka",
                                                    "fontsize": 12.0,
                                                    "id": "obj-60",
                                                    "maxclass": "message",
                                                    "numinlets": 2,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "" ],
                                                    "patching_rect": [ 124.0, 127.0, 95.0, 23.0 ],
                                                    "text": "cantchange $1"
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-71",
                                                    "index": 1,
                                                    "maxclass": "inlet",
                                                    "numinlets": 0,
                                                    "numoutlets": 1,
                                                    "outlettype": [ "" ],
                                                    "patching_rect": [ 82.5, 40.0, 25.0, 25.0 ]
                                                }
                                            },
                                            {
                                                "box": {
                                                    "comment": "",
                                                    "id": "obj-72",
                                                    "index": 1,
                                                    "maxclass": "outlet",
                                                    "numinlets": 1,
                                                    "numoutlets": 0,
                                                    "patching_rect": [ 82.0, 209.0, 25.0, 25.0 ]
                                                }
                                            }
                                        ],
                                        "lines": [
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-72", 0 ],
                                                    "source": [ "obj-4", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-72", 0 ],
                                                    "source": [ "obj-60", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-60", 0 ],
                                                    "source": [ "obj-70", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-4", 0 ],
                                                    "order": 1,
                                                    "source": [ "obj-71", 0 ]
                                                }
                                            },
                                            {
                                                "patchline": {
                                                    "destination": [ "obj-70", 0 ],
                                                    "order": 0,
                                                    "source": [ "obj-71", 0 ]
                                                }
                                            }
                                        ],
                                        "bgfillcolor_type": "color",
                                        "bgfillcolor_color1": [ 0.301961, 0.301961, 0.301961, 1 ],
                                        "bgfillcolor_color2": [ 0.2, 0.2, 0.2, 1 ],
                                        "bgfillcolor_color": [ 0.2, 0.2, 0.2, 1.0 ]
                                    },
                                    "patching_rect": [ 334.0, 140.0, 82.0, 22.0 ],
                                    "saved_object_attributes": {
                                        "fontname": "Osaka",
                                        "fontsize": 10.0
                                    },
                                    "text": "p appearance"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-112",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 427.0, 140.0, 52.0, 22.0 ],
                                    "text": "gate 1 1"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-30",
                                    "maxclass": "newobj",
                                    "numinlets": 6,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 471.0, 276.0, 94.0, 22.0 ],
                                    "text": "scale -1. 1. 0. 2."
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-7",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 74.0, 119.0, 31.0, 22.0 ],
                                    "text": "sig~"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-6",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 31.0, 83.0, 31.0, 22.0 ],
                                    "text": "sig~"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-5",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 74.0, 83.0, 246.0, 22.0 ],
                                    "restore": [ 0.41856186074576873 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr @bindto parent::randpitch- @invisible 1",
                                    "varname": "u322010437[2]"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-178",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 3,
                                    "outlettype": [ "", "", "" ],
                                    "patching_rect": [ 31.0, 34.0, 222.0, 22.0 ],
                                    "restore": [ 0.7716535433070866 ],
                                    "saved_object_attributes": {
                                        "parameter_enable": 0,
                                        "parameter_mappable": 0
                                    },
                                    "text": "pattr @bindto parent::pitch- @invisible 1",
                                    "varname": "u322010437[1]"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-197",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 31.0, 336.0, 41.0, 22.0 ],
                                    "text": "in0 $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-198",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 31.0, 371.0, 171.0, 22.0 ],
                                    "text": "pattrforward parent::multislider"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-127",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 31.0, 247.0, 33.0, 22.0 ],
                                    "text": "-~ 1."
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-126",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 31.0, 216.0, 29.5, 22.0 ],
                                    "text": "+~"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-124",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "float" ],
                                    "patching_rect": [ 31.0, 276.0, 81.0, 22.0 ],
                                    "text": "snapshot~ 30"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-123",
                                    "maxclass": "newobj",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 74.0, 169.0, 29.5, 22.0 ],
                                    "text": "*~"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontsize": 12.0,
                                    "id": "obj-122",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "signal" ],
                                    "patching_rect": [ 110.0, 119.0, 44.0, 22.0 ],
                                    "text": "noise~"
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-112", 1 ],
                                    "midpoints": [ 518.5, 126.0, 469.5, 126.0 ],
                                    "source": [ "obj-1", 1 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-54", 0 ],
                                    "midpoints": [ 507.5, 93.0, 507.5, 93.0 ],
                                    "source": [ "obj-1", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-197", 0 ],
                                    "midpoints": [ 436.5, 321.0, 40.5, 321.0 ],
                                    "order": 1,
                                    "source": [ "obj-112", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-30", 0 ],
                                    "midpoints": [ 436.5, 261.0, 480.5, 261.0 ],
                                    "order": 0,
                                    "source": [ "obj-112", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-123", 1 ],
                                    "midpoints": [ 119.5, 156.0, 94.0, 156.0 ],
                                    "source": [ "obj-122", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-126", 1 ],
                                    "midpoints": [ 83.5, 201.0, 51.0, 201.0 ],
                                    "source": [ "obj-123", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-197", 0 ],
                                    "midpoints": [ 40.5, 300.0, 40.5, 300.0 ],
                                    "source": [ "obj-124", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-127", 0 ],
                                    "midpoints": [ 40.5, 240.0, 40.5, 240.0 ],
                                    "source": [ "obj-126", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-124", 0 ],
                                    "midpoints": [ 40.5, 270.0, 40.5, 270.0 ],
                                    "source": [ "obj-127", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-6", 0 ],
                                    "midpoints": [ 40.5, 57.0, 40.5, 57.0 ],
                                    "source": [ "obj-178", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-198", 0 ],
                                    "midpoints": [ 40.5, 360.0, 40.5, 360.0 ],
                                    "source": [ "obj-197", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-147", 0 ],
                                    "midpoints": [ 480.5, 300.0, 480.5, 300.0 ],
                                    "source": [ "obj-30", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-1", 0 ],
                                    "midpoints": [ 507.5, 57.0, 507.5, 57.0 ],
                                    "source": [ "obj-44", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-7", 0 ],
                                    "midpoints": [ 83.5, 108.0, 83.5, 108.0 ],
                                    "source": [ "obj-5", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-126", 0 ],
                                    "midpoints": [ 40.5, 108.0, 40.5, 108.0 ],
                                    "source": [ "obj-6", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-123", 0 ],
                                    "midpoints": [ 83.5, 144.0, 83.5, 144.0 ],
                                    "source": [ "obj-7", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-167", 0 ],
                                    "midpoints": [ 343.5, 165.0, 343.5, 165.0 ],
                                    "source": [ "obj-79", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-112", 0 ],
                                    "midpoints": [ 436.5, 66.0, 436.5, 66.0 ],
                                    "source": [ "obj-8", 0 ]
                                }
                            }
                        ],
                        "bgfillcolor_type": "gradient",
                        "bgfillcolor_color1": [ 0.172137149796092, 0.172137100044002, 0.172137113045018, 1 ],
                        "bgfillcolor_color2": [ 0.172137149796092, 0.172137100044002, 0.172137113045018, 1 ],
                        "bgfillcolor_color": [ 0.172137149796092, 0.172137100044002, 0.172137113045018, 1 ]
                    },
                    "patching_rect": [ 1918.0, 177.0, 105.0, 22.0 ],
                    "text": "p multislider–pitch",
                    "varname": "multislider–pitch"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "format": 6,
                    "id": "obj-55",
                    "maxclass": "flonum",
                    "maximum": 1.0,
                    "minimum": 0.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 185.0, 437.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 16.0, 288.0, 50.0, 22.0 ],
                    "varname": "pos-"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-64",
                    "maxclass": "flonum",
                    "maximum": 1.0,
                    "minimum": 0.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1347.865551352501, 198.73948395252228, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 251.0, 358.0, 49.0, 22.0 ],
                    "varname": "spread-"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-63",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 883.0, 238.0, 99.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 251.0, 87.0, 99.0, 20.0 ],
                    "text": "number of grains"
                }
            },
            {
                "box": {
                    "allowdrag": 0,
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-62",
                    "items": [ 1, ",", 2, ",", 4, ",", 6, ",", 8, ",", 12, ",", 16, ",", 24, ",", 32, ",", 48, ",", 64 ],
                    "maxclass": "umenu",
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "int", "", "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 907.0, 260.0, 50.0, 22.0 ],
                    "pattrmode": 1,
                    "presentation": 1,
                    "presentation_rect": [ 197.0, 86.0, 50.0, 22.0 ]
                }
            },
            {
                "box": {
                    "dontreplace": 1,
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-129",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2172.0, 317.0, 84.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 361.7647209763527, 117.0, 118.78773111104965, 22.0 ]
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-128",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "int" ],
                    "patching_rect": [ 2237.0, 281.0, 55.0, 22.0 ],
                    "text": "strippath"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-121",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 3145.0, 40.0, 81.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 424.5105085372925, 393.0, 81.0, 20.0 ],
                    "text": "record to disk"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-118",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 271.0, 48.0, 89.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 32.323426842689514, 13.5, 89.0, 20.0 ],
                    "text": "playback mode"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-97",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 139.0, 335.0, 39.0, 22.0 ],
                    "text": "/ 100."
                }
            },
            {
                "box": {
                    "id": "obj-96",
                    "maxclass": "number",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 216.0, 182.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 16.0, 86.0, 50.0, 22.0 ],
                    "varname": "speed-"
                }
            },
            {
                "box": {
                    "fontface": 3,
                    "fontsize": 12.0,
                    "hint": "",
                    "id": "obj-91",
                    "maxclass": "textbutton",
                    "mode": 1,
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "", "", "int" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 191.0, 39.0, 75.0, 37.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 15.0, 36.0, 123.64685368537903, 40.0 ],
                    "rounded": 5.0,
                    "text": "loop play",
                    "texton": "pause",
                    "varname": "mode-"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-36",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 3115.0, 298.0, 88.0, 22.0 ],
                    "text": "mc.sfrecord~ 2"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "format": 6,
                    "id": "obj-27",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 649.0, 407.5, 78.0, 22.0 ],
                    "varname": "curpos"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-10",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 649.0, 378.0, 74.0, 22.0 ],
                    "text": "snapshot~ 5"
                }
            },
            {
                "box": {
                    "id": "obj-125",
                    "maxclass": "led",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 3115.0, 38.0, 24.0, 24.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 395.5105085372925, 391.0, 24.0, 24.0 ],
                    "varname": "record[1]"
                }
            },
            {
                "box": {
                    "bgcolor": [ 0.125, 0.125, 0.125, 0.0 ],
                    "id": "obj-105",
                    "maxclass": "multislider",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1628.0, 255.0, 444.0, 120.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 15.000000715255737, 159.54730343818665, 474.0, 110.0 ],
                    "varname": "multislider"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-103",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2321.0, 480.0, 51.0, 22.0 ],
                    "text": "s length"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-82",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2226.0, 78.0, 99.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 372.35295671224594, 45.882354855537415, 99.0, 20.0 ],
                    "text": "drag & drop here"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-83",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2121.0, 281.0, 95.0, 22.0 ],
                    "text": "prepend replace"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-84",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2321.0, 208.0, 143.0, 22.0 ],
                    "text": "gettimescale, getduration"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-85",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2479.0, 209.0, 80.0, 22.0 ],
                    "text": "prepend read"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-86",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 4,
                    "outlettype": [ "bang", "bang", "", "" ],
                    "patching_rect": [ 2216.0, 146.0, 51.0, 22.0 ],
                    "text": "t b b s s"
                }
            },
            {
                "box": {
                    "border": 4.0,
                    "id": "obj-87",
                    "maxclass": "dropfile",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "" ],
                    "patching_rect": [ 2216.0, 62.0, 122.0, 55.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 362.94119161367416, 35.8823544383049, 117.61126047372818, 40.000001668930054 ],
                    "rounded": 0.0
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-90",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 2321.0, 378.0, 29.5, 22.0 ],
                    "text": "/ 1."
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-93",
                    "maxclass": "newobj",
                    "numinlets": 3,
                    "numoutlets": 3,
                    "outlettype": [ "", "", "" ],
                    "patching_rect": [ 2321.0, 330.0, 137.0, 22.0 ],
                    "text": "route duration timescale"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-94",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "jit_matrix", "" ],
                    "patching_rect": [ 2321.0, 281.0, 138.0, 22.0 ],
                    "text": "jit.qt.movie @autostart 0"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-76",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 729.0, 408.5, 118.0, 20.0 ],
                    "text": "current position (ms)"
                }
            },
            {
                "box": {
                    "id": "obj-65",
                    "maxclass": "button",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 2168.0, 479.0, 24.0, 24.0 ],
                    "varname": "button"
                }
            },
            {
                "box": {
                    "fontsize": 12.0,
                    "id": "obj-58",
                    "maxclass": "preset",
                    "numinlets": 1,
                    "numoutlets": 5,
                    "outlettype": [ "preset", "int", "preset", "int", "" ],
                    "patching_rect": [ 893.0, 48.0, 101.0, 41.0 ],
                    "pattrstorage": "granulator",
                    "presentation": 1,
                    "presentation_rect": [ 197.1098120212555, 35.83814764022827, 100.57802724838257, 40.462424755096436 ],
                    "varname": "preset"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-56",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 267.0, 183.0, 114.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 70.0, 87.0, 114.0, 20.0 ],
                    "text": "playback speed (%)"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-31",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 40.0, 366.0, 31.0, 22.0 ],
                    "text": "sig~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "format": 6,
                    "id": "obj-28",
                    "maxclass": "flonum",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 139.0, 369.0, 50.0, 22.0 ],
                    "varname": "number[5]"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-23",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 124.0, 484.0, 34.0, 22.0 ],
                    "text": "*~ 1."
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-21",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 40.0, 335.0, 49.0, 22.0 ],
                    "text": "r length"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-20",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 70.0, 437.0, 56.0, 22.0 ],
                    "text": "!/~ 1000."
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-15",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 124.0, 524.0, 52.0, 22.0 ],
                    "text": "phasor~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-53",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1347.865551352501, 177.73108184337616, 45.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 304.0, 359.0, 45.0, 20.0 ],
                    "text": "spread"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-52",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1270.9831928014755, 216.05042159557343, 31.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 305.0, 394.0, 31.0, 20.0 ],
                    "text": "gain"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-50",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 990.0, 270.0, 87.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 69.27778029441833, 357.69444447755814, 87.0, 20.0 ],
                    "text": "grain size (ms)"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-49",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1164.0, 270.0, 78.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 304.0, 324.0, 78.0, 20.0 ],
                    "text": "random pitch"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-48",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1098.0, 270.0, 34.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 304.0, 289.0, 34.0, 20.0 ],
                    "text": "pitch"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-47",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 722.0, 93.85713732242584, 94.0, 20.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 68.77778029441833, 322.69444447755814, 94.0, 20.0 ],
                    "text": "random position"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-40",
                    "maxclass": "flonum",
                    "minimum": 0.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1263.0, 240.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 251.0, 393.0, 49.0, 22.0 ],
                    "varname": "gain-"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-39",
                    "maxclass": "flonum",
                    "minimum": 1.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1007.0, 294.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 16.5, 357.0, 50.0, 22.0 ],
                    "varname": "grainsize-"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-38",
                    "maxclass": "flonum",
                    "minimum": 0.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1177.0, 294.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 251.0, 323.0, 49.0, 22.0 ],
                    "varname": "randpitch-"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-34",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 649.0, 329.0, 29.5, 22.0 ],
                    "text": "+~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-32",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 660.0, 279.0, 30.0, 22.0 ],
                    "text": "*~ 5"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-33",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 612.0, 219.0, 44.0, 22.0 ],
                    "text": "noise~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-29",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1263.0, 328.0, 44.0, 22.0 ],
                    "text": "sig~ 1."
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-24",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1348.0, 328.0, 43.0, 22.0 ],
                    "text": "+~ 0.5"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-13",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1348.0, 294.0, 29.5, 22.0 ],
                    "text": "*~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-25",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1385.0, 240.0, 44.0, 22.0 ],
                    "text": "noise~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-22",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1007.0, 328.0, 58.0, 22.0 ],
                    "text": "sig~ 100."
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-19",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1177.0, 328.0, 31.0, 22.0 ],
                    "text": "sig~"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-18",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 1093.0, 328.0, 44.0, 22.0 ],
                    "text": "sig~ 1."
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-14",
                    "maxclass": "flonum",
                    "minimum": 0.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1093.0, 294.0, 50.0, 22.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 251.0, 288.0, 49.0, 22.0 ],
                    "varname": "pitch-"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-3",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "float", "bang" ],
                    "patching_rect": [ 2121.0, 443.0, 66.0, 22.0 ],
                    "text": "buffer~ buf"
                }
            },
            {
                "box": {
                    "id": "obj-51",
                    "maxclass": "ezdac~",
                    "numinlets": 2,
                    "numoutlets": 0,
                    "patching_rect": [ 1371.2065630555153, 629.6032815277576, 57.79343694448471, 57.79343694448471 ],
                    "presentation": 1,
                    "presentation_rect": [ 395.5105085372925, 286.0, 94.0, 94.0 ]
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-108",
                    "maxclass": "newobj",
                    "numinlets": 8,
                    "numoutlets": 2,
                    "outlettype": [ "signal", "signal" ],
                    "patching_rect": [ 922.0, 377.0, 616.0, 22.0 ],
                    "text": "poly~ p_grain 8",
                    "varname": "poly~_AA"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-45",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 922.0, 328.0, 59.0, 22.0 ],
                    "text": "voices $1"
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-106",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2676.0, 149.0, 29.5, 22.0 ],
                    "text": "0"
                }
            },
            {
                "box": {
                    "format": 6,
                    "id": "obj-109",
                    "maxclass": "flonum",
                    "minimum": 0.0,
                    "numinlets": 1,
                    "numoutlets": 2,
                    "outlettype": [ "", "bang" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 2778.0, 341.0, 50.0, 22.0 ]
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-110",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2716.0, 402.0, 35.0, 22.0 ],
                    "text": "set 0"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-11",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 2,
                    "outlettype": [ "bang", "" ],
                    "patching_rect": [ 2716.0, 379.0, 34.0, 22.0 ],
                    "text": "sel 1"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-115",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "patching_rect": [ 2716.0, 340.0, 36.0, 22.0 ],
                    "text": "== 1."
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontsize": 12.0,
                    "id": "obj-117",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "", "int", "int" ],
                    "patching_rect": [ 2716.0, 360.0, 48.0, 22.0 ],
                    "text": "change"
                }
            },
            {
                "box": {
                    "fontface": 0,
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-2",
                    "maxclass": "number~",
                    "mode": 2,
                    "numinlets": 2,
                    "numoutlets": 2,
                    "outlettype": [ "signal", "float" ],
                    "patching_rect": [ 2676.0, 292.0, 59.0, 22.0 ],
                    "sig": 0.0
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-132",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2640.0, 102.0, 31.0, 20.0 ],
                    "text": "loop"
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-135",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2375.0, 466.0, 91.0, 20.0 ],
                    "text": "buffer size (ms)"
                }
            },
            {
                "box": {
                    "id": "obj-140",
                    "maxclass": "toggle",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "int" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 2614.0, 100.0, 24.0, 24.0 ],
                    "varname": "loop-rec-"
                }
            },
            {
                "box": {
                    "fontface": 3,
                    "fontsize": 12.0,
                    "hint": "",
                    "id": "obj-142",
                    "legacytextcolor": 1,
                    "maxclass": "textbutton",
                    "mode": 1,
                    "numinlets": 1,
                    "numoutlets": 3,
                    "outlettype": [ "", "", "int" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 2676.0, 191.0, 75.0, 37.0 ],
                    "presentation": 1,
                    "presentation_rect": [ 501.55100855231285, 33.0, 123.64685368537903, 40.0 ],
                    "rounded": 5.0,
                    "text": "record",
                    "texton": "record",
                    "textoncolor": [ 1.0, 0.0, 0.0, 1.0 ],
                    "varname": "record"
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-143",
                    "maxclass": "newobj",
                    "numinlets": 3,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 2676.0, 253.0, 70.0, 22.0 ],
                    "text": "record~ buf"
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-8",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "signal" ],
                    "patching_rect": [ 2824.0, 192.0, 34.0, 22.0 ],
                    "text": "*~ 1."
                }
            },
            {
                "box": {
                    "id": "obj-145",
                    "maxclass": "meter~",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "float" ],
                    "patching_rect": [ 2824.0, 256.0, 68.78306984901428, 15.873016119003296 ]
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-148",
                    "maxclass": "message",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 2614.0, 149.0, 48.0, 22.0 ],
                    "text": "loop $1"
                }
            },
            {
                "box": {
                    "fgcolor": [ 1.0, 1.0, 1.0, 0.4 ],
                    "id": "obj-100",
                    "maxclass": "rslider",
                    "numinlets": 2,
                    "numoutlets": 2,
                    "outlettype": [ "", "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 1628.0, 255.0, 444.7297802567482, 118.33783257007599 ],
                    "presentation": 1,
                    "presentation_rect": [ 15.555556297302246, 157.88063669204712, 474.51050782203674, 113.23048734664917 ],
                    "size": 3009.0,
                    "thickness": 100.0,
                    "varname": "rslider"
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-98",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 875.0, 17.0, 728.0, 144.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-144",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 17.0, 17.0, 419.0, 773.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-141",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 441.0, 17.0, 430.0, 773.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-136",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 875.0, 166.0, 728.0, 624.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-74",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 3080.0, 17.0, 180.0, 522.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-257",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2589.0, 17.0, 484.0, 522.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-256",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 2098.0, 17.0, 487.0, 522.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            },
            {
                "box": {
                    "angle": 270.0,
                    "background": 1,
                    "bgcolor": [ 0.301961, 0.301961, 0.301961, 1.0 ],
                    "border": 1,
                    "bordercolor": [ 0.0, 0.0, 0.0, 1.0 ],
                    "id": "obj-99",
                    "maxclass": "panel",
                    "mode": 0,
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 1608.0, 17.0, 486.0, 522.0 ],
                    "proportion": 0.5,
                    "rounded": 0
                }
            }
        ],
        "lines": [
            {
                "patchline": {
                    "destination": [ "obj-3", 0 ],
                    "midpoints": [ 2160.5, 429.0, 2130.5, 429.0 ],
                    "source": [ "obj-1", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-27", 0 ],
                    "midpoints": [ 658.5, 402.0, 658.5, 402.0 ],
                    "source": [ "obj-10", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 7 ],
                    "midpoints": [ 1528.5, 351.0, 1528.5, 351.0 ],
                    "source": [ "obj-104", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-142", 0 ],
                    "midpoints": [ 2685.5, 174.0, 2685.5, 174.0 ],
                    "source": [ "obj-106", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-13", 0 ],
                    "midpoints": [ 1357.5, 264.0, 1357.5, 264.0 ],
                    "source": [ "obj-107", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-88", 3 ],
                    "midpoints": [ 1528.5, 480.0, 1135.5, 480.0 ],
                    "source": [ "obj-108", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-88", 2 ],
                    "midpoints": [ 931.5, 411.0, 1067.5, 411.0 ],
                    "source": [ "obj-108", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-110", 0 ],
                    "midpoints": [ 2725.5, 402.0, 2725.5, 402.0 ],
                    "source": [ "obj-11", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-142", 0 ],
                    "midpoints": [ 2725.5, 426.0, 2661.0, 426.0, 2661.0, 186.0, 2685.5, 186.0 ],
                    "source": [ "obj-110", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-96", 0 ],
                    "midpoints": [ 224.5, 309.0, 201.0, 309.0, 201.0, 177.0, 225.5, 177.0 ],
                    "source": [ "obj-111", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-77", 0 ],
                    "midpoints": [ 1927.5, 165.0, 1927.5, 165.0 ],
                    "source": [ "obj-113", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-117", 0 ],
                    "midpoints": [ 2725.5, 363.0, 2725.5, 363.0 ],
                    "source": [ "obj-115", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-111", 0 ],
                    "midpoints": [ 200.5, 270.0, 224.5, 270.0 ],
                    "source": [ "obj-116", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-119", 0 ],
                    "midpoints": [ 213.0, 126.0, 225.5, 126.0 ],
                    "source": [ "obj-116", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-11", 0 ],
                    "midpoints": [ 2725.5, 384.0, 2725.5, 384.0 ],
                    "source": [ "obj-117", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-96", 0 ],
                    "midpoints": [ 225.5, 156.0, 225.5, 156.0 ],
                    "source": [ "obj-119", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-46", 0 ],
                    "midpoints": [ 2358.5, 87.0, 2358.5, 87.0 ],
                    "source": [ "obj-12", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-111", 1 ],
                    "midpoints": [ 240.5, 255.0, 240.5, 255.0 ],
                    "source": [ "obj-120", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-91", 0 ],
                    "midpoints": [ 266.5, 318.0, 177.0, 318.0, 177.0, 36.0, 200.5, 36.0 ],
                    "source": [ "obj-123", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-126", 0 ],
                    "midpoints": [ 266.5, 255.0, 266.5, 255.0 ],
                    "source": [ "obj-124", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-71", 0 ],
                    "midpoints": [ 3124.5, 63.0, 3124.5, 63.0 ],
                    "source": [ "obj-125", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-123", 0 ],
                    "midpoints": [ 266.5, 282.0, 266.5, 282.0 ],
                    "source": [ "obj-126", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-130", 0 ],
                    "midpoints": [ 86.5, 642.0, 86.5, 642.0 ],
                    "source": [ "obj-127", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-129", 1 ],
                    "midpoints": [ 2246.5, 306.0, 2246.5, 306.0 ],
                    "source": [ "obj-128", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-24", 0 ],
                    "midpoints": [ 1357.5, 318.0, 1357.5, 318.0 ],
                    "source": [ "obj-13", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-133", 1 ],
                    "midpoints": [ 86.5, 702.0, 60.0, 702.0 ],
                    "source": [ "obj-130", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-35", 0 ],
                    "midpoints": [ 680.508363366127, 195.0, 680.5, 195.0 ],
                    "source": [ "obj-131", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-181", 0 ],
                    "midpoints": [ 49.5, 738.0, 49.5, 738.0 ],
                    "source": [ "obj-133", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-130", 1 ],
                    "midpoints": [ 161.5, 651.0, 97.0, 651.0 ],
                    "source": [ "obj-138", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-18", 0 ],
                    "midpoints": [ 1102.5, 318.0, 1102.5, 318.0 ],
                    "source": [ "obj-14", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-106", 0 ],
                    "midpoints": [ 2623.5, 135.0, 2685.5, 135.0 ],
                    "order": 0,
                    "source": [ "obj-140", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-148", 0 ],
                    "midpoints": [ 2623.5, 126.0, 2623.5, 126.0 ],
                    "order": 1,
                    "source": [ "obj-140", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-143", 0 ],
                    "midpoints": [ 2685.5, 231.0, 2685.5, 231.0 ],
                    "source": [ "obj-142", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-2", 0 ],
                    "midpoints": [ 2685.5, 276.0, 2685.5, 276.0 ],
                    "source": [ "obj-143", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-15", 1 ],
                    "midpoints": [ 194.5, 510.0, 166.5, 510.0 ],
                    "source": [ "obj-147", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-143", 0 ],
                    "midpoints": [ 2623.5, 240.0, 2685.5, 240.0 ],
                    "source": [ "obj-148", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-127", 0 ],
                    "midpoints": [ 133.5, 561.0, 86.5, 561.0 ],
                    "order": 1,
                    "source": [ "obj-15", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-205", 0 ],
                    "midpoints": [ 133.5, 561.0, 223.5, 561.0 ],
                    "order": 0,
                    "source": [ "obj-15", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-70", 1 ],
                    "midpoints": [ 586.8066889047623, 126.0, 691.008363366127, 126.0 ],
                    "source": [ "obj-150", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-70", 0 ],
                    "midpoints": [ 576.3066889047623, 126.0, 680.508363366127, 126.0 ],
                    "source": [ "obj-150", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-164", 0 ],
                    "midpoints": [ 1185.5, 102.0, 1102.5, 102.0 ],
                    "source": [ "obj-151", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-8", 0 ],
                    "midpoints": [ 2849.5, 177.0, 2833.5, 177.0 ],
                    "source": [ "obj-152", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-8", 0 ],
                    "midpoints": [ 2833.5, 87.0, 2833.5, 87.0 ],
                    "source": [ "obj-152", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-164", 0 ],
                    "midpoints": [ 1102.5, 81.0, 1102.5, 81.0 ],
                    "source": [ "obj-153", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-86", 0 ],
                    "midpoints": [ 2443.5, 132.0, 2225.5, 132.0 ],
                    "source": [ "obj-154", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-168", 0 ],
                    "midpoints": [ 931.5, 450.0, 931.5, 450.0 ],
                    "source": [ "obj-155", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-154", 0 ],
                    "midpoints": [ 2443.5, 87.0, 2443.5, 87.0 ],
                    "source": [ "obj-156", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-160", 0 ],
                    "midpoints": [ 1025.5, 81.0, 1025.5, 81.0 ],
                    "source": [ "obj-159", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-113", 0 ],
                    "midpoints": [ 1927.5, 132.0, 1927.5, 132.0 ],
                    "source": [ "obj-16", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-173", 0 ],
                    "midpoints": [ 1637.5, 96.0, 1637.5, 96.0 ],
                    "source": [ "obj-161", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-173", 1 ],
                    "midpoints": [ 1695.5, 117.0, 1686.5, 117.0 ],
                    "source": [ "obj-162", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-43", 0 ],
                    "midpoints": [ 1637.5, 201.0, 1637.5, 201.0 ],
                    "source": [ "obj-165", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-167", 0 ],
                    "midpoints": [ 998.5, 450.0, 998.5, 450.0 ],
                    "source": [ "obj-166", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-88", 1 ],
                    "midpoints": [ 998.5, 489.0, 999.5, 489.0 ],
                    "source": [ "obj-167", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-88", 0 ],
                    "midpoints": [ 931.5, 483.0, 931.5, 483.0 ],
                    "source": [ "obj-168", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-29", 0 ],
                    "midpoints": [ 1272.5, 318.0, 1272.5, 318.0 ],
                    "source": [ "obj-17", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-165", 0 ],
                    "midpoints": [ 1637.5, 153.0, 1637.5, 153.0 ],
                    "source": [ "obj-173", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-150", 0 ],
                    "midpoints": [ 576.3066889047623, 72.0, 576.3066889047623, 72.0 ],
                    "source": [ "obj-176", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 2 ],
                    "midpoints": [ 1102.5, 351.0, 1102.0714285714287, 351.0 ],
                    "source": [ "obj-18", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-185", 0 ],
                    "midpoints": [ 478.5, 303.0, 478.5, 303.0 ],
                    "order": 1,
                    "source": [ "obj-183", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-34", 0 ],
                    "midpoints": [ 478.5, 315.0, 658.5, 315.0 ],
                    "order": 0,
                    "source": [ "obj-183", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-41", 0 ],
                    "midpoints": [ 931.5, 546.0, 931.5, 546.0 ],
                    "source": [ "obj-184", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-9", 0 ],
                    "midpoints": [ 478.5, 354.0, 478.5, 354.0 ],
                    "source": [ "obj-185", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-308", 0 ],
                    "midpoints": [ 931.5, 744.0, 931.5, 744.0 ],
                    "source": [ "obj-187", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 3 ],
                    "midpoints": [ 1186.5, 351.0, 1187.357142857143, 351.0 ],
                    "source": [ "obj-19", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-109", 0 ],
                    "midpoints": [ 2725.5, 327.0, 2787.5, 327.0 ],
                    "order": 0,
                    "source": [ "obj-2", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-115", 0 ],
                    "midpoints": [ 2725.5, 315.0, 2725.5, 315.0 ],
                    "order": 1,
                    "source": [ "obj-2", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-23", 0 ],
                    "midpoints": [ 79.5, 471.0, 133.5, 471.0 ],
                    "source": [ "obj-20", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-31", 0 ],
                    "midpoints": [ 49.5, 360.0, 49.5, 360.0 ],
                    "source": [ "obj-21", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-23", 1 ],
                    "midpoints": [ 148.5, 462.0, 148.5, 462.0 ],
                    "source": [ "obj-210", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-45", 0 ],
                    "midpoints": [ 931.5, 318.0, 931.5, 318.0 ],
                    "source": [ "obj-213", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-89", 0 ],
                    "midpoints": [ 2330.5, 432.0, 2330.5, 432.0 ],
                    "source": [ "obj-214", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-210", 1 ],
                    "midpoints": [ 226.0, 423.0, 159.0, 423.0 ],
                    "source": [ "obj-215", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-210", 0 ],
                    "midpoints": [ 215.5, 423.0, 148.5, 423.0 ],
                    "source": [ "obj-215", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-127", 1 ],
                    "midpoints": [ 135.5, 606.0, 97.0, 606.0 ],
                    "source": [ "obj-216", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-215", 0 ],
                    "midpoints": [ 215.5, 360.0, 215.5, 360.0 ],
                    "source": [ "obj-217", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 1 ],
                    "midpoints": [ 1016.5, 351.0, 1016.7857142857143, 351.0 ],
                    "source": [ "obj-22", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-235", 0 ],
                    "midpoints": [ 1971.5, 456.0, 1971.5, 456.0 ],
                    "source": [ "obj-220", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-220", 0 ],
                    "midpoints": [ 1971.5, 426.0, 1971.5, 426.0 ],
                    "source": [ "obj-225", 2 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-231", 0 ],
                    "midpoints": [ 1730.5, 426.0, 1730.5, 426.0 ],
                    "source": [ "obj-225", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-233", 0 ],
                    "midpoints": [ 1851.0, 426.0, 1850.5, 426.0 ],
                    "source": [ "obj-225", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-15", 0 ],
                    "midpoints": [ 133.5, 507.0, 133.5, 507.0 ],
                    "source": [ "obj-23", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 5 ],
                    "midpoints": [ 1357.5, 351.0, 1357.9285714285716, 351.0 ],
                    "source": [ "obj-24", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-80", 0 ],
                    "midpoints": [ 680.508363366127, 69.0, 680.508363366127, 69.0 ],
                    "source": [ "obj-241", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-255", 0 ],
                    "midpoints": [ 2885.5, 123.0, 2885.5, 123.0 ],
                    "source": [ "obj-249", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-13", 1 ],
                    "midpoints": [ 1394.5, 279.0, 1368.0, 279.0 ],
                    "source": [ "obj-25", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-249", 0 ],
                    "midpoints": [ 2885.5, 87.0, 2885.5, 87.0 ],
                    "source": [ "obj-252", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-140", 0 ],
                    "midpoints": [ 2623.5, 87.0, 2623.5, 87.0 ],
                    "source": [ "obj-253", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-8", 1 ],
                    "midpoints": [ 2885.5, 189.0, 2848.5, 189.0 ],
                    "source": [ "obj-255", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-259", 0 ],
                    "midpoints": [ 3124.5, 132.0, 3124.5, 132.0 ],
                    "source": [ "obj-258", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-261", 0 ],
                    "midpoints": [ 3172.5, 132.0, 3172.5, 132.0 ],
                    "source": [ "obj-258", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-262", 0 ],
                    "midpoints": [ 3124.5, 216.0, 3124.5, 216.0 ],
                    "source": [ "obj-259", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-37", 0 ],
                    "midpoints": [ 2130.5, 231.0, 2130.5, 231.0 ],
                    "source": [ "obj-26", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-125", 0 ],
                    "midpoints": [ 3172.5, 168.0, 3102.0, 168.0, 3102.0, 33.0, 3124.5, 33.0 ],
                    "source": [ "obj-261", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-36", 0 ],
                    "midpoints": [ 3135.0, 285.0, 3124.5, 285.0 ],
                    "source": [ "obj-262", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-36", 0 ],
                    "midpoints": [ 3124.5, 246.0, 3124.5, 246.0 ],
                    "source": [ "obj-262", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-66", 0 ],
                    "midpoints": [ 658.5, 432.0, 658.5, 432.0 ],
                    "source": [ "obj-27", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-210", 0 ],
                    "midpoints": [ 148.5, 393.0, 148.5, 393.0 ],
                    "source": [ "obj-28", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 4 ],
                    "midpoints": [ 1272.5, 351.0, 1272.642857142857, 351.0 ],
                    "source": [ "obj-29", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-65", 0 ],
                    "midpoints": [ 2177.5, 468.0, 2177.5, 468.0 ],
                    "source": [ "obj-3", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-133", 0 ],
                    "midpoints": [ 49.5, 390.0, 49.5, 390.0 ],
                    "order": 1,
                    "source": [ "obj-31", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-20", 0 ],
                    "midpoints": [ 49.5, 423.0, 79.5, 423.0 ],
                    "order": 0,
                    "source": [ "obj-31", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 6 ],
                    "midpoints": [ 1443.5, 351.0, 1443.2142857142858, 351.0 ],
                    "source": [ "obj-316", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-320", 0 ],
                    "midpoints": [ 1443.5, 222.0, 1443.5, 222.0 ],
                    "source": [ "obj-319", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-34", 1 ],
                    "midpoints": [ 669.5, 303.0, 669.0, 303.0 ],
                    "source": [ "obj-32", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-316", 0 ],
                    "midpoints": [ 1443.5, 264.0, 1443.5, 264.0 ],
                    "source": [ "obj-320", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-32", 0 ],
                    "midpoints": [ 621.5, 264.0, 669.5, 264.0 ],
                    "source": [ "obj-33", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-10", 0 ],
                    "midpoints": [ 658.5, 354.0, 658.5, 354.0 ],
                    "order": 1,
                    "source": [ "obj-34", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 0 ],
                    "midpoints": [ 658.5, 363.0, 931.5, 363.0 ],
                    "order": 0,
                    "source": [ "obj-34", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-32", 1 ],
                    "midpoints": [ 680.5, 243.0, 680.5, 243.0 ],
                    "source": [ "obj-35", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-19", 0 ],
                    "midpoints": [ 1186.5, 318.0, 1186.5, 318.0 ],
                    "source": [ "obj-38", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-22", 0 ],
                    "midpoints": [ 1016.5, 318.0, 1016.5, 318.0 ],
                    "source": [ "obj-39", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-12", 0 ],
                    "midpoints": [ 2358.5, 57.0, 2358.5, 57.0 ],
                    "source": [ "obj-4", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-17", 0 ],
                    "midpoints": [ 1272.5, 264.0, 1272.5, 264.0 ],
                    "source": [ "obj-40", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-175", 0 ],
                    "midpoints": [ 931.5, 615.0, 1138.5, 615.0 ],
                    "order": 0,
                    "source": [ "obj-41", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-186", 0 ],
                    "midpoints": [ 931.5, 603.0, 931.5, 603.0 ],
                    "order": 2,
                    "source": [ "obj-41", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-79", 0 ],
                    "midpoints": [ 931.5, 651.0, 1009.5, 651.0 ],
                    "order": 1,
                    "source": [ "obj-41", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-108", 0 ],
                    "midpoints": [ 931.5, 351.0, 931.5, 351.0 ],
                    "source": [ "obj-45", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-86", 0 ],
                    "midpoints": [ 2358.5, 132.0, 2225.5, 132.0 ],
                    "source": [ "obj-46", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-75", 0 ],
                    "midpoints": [ 478.5, 627.0, 478.5, 627.0 ],
                    "source": [ "obj-54", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-147", 0 ],
                    "midpoints": [ 194.5, 462.0, 194.5, 462.0 ],
                    "source": [ "obj-55", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-189", 0 ],
                    "midpoints": [ 478.5, 462.0, 478.5, 462.0 ],
                    "source": [ "obj-60", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-213", 0 ],
                    "midpoints": [ 932.0, 285.0, 931.5, 285.0 ],
                    "source": [ "obj-62", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-107", 0 ],
                    "midpoints": [ 1357.365551352501, 222.0, 1357.5, 222.0 ],
                    "source": [ "obj-64", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-9", 1 ],
                    "midpoints": [ 568.5, 363.0, 489.0, 363.0 ],
                    "source": [ "obj-68", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-172", 0 ],
                    "midpoints": [ 1977.9918681144713, 390.0, 2027.5, 390.0 ],
                    "source": [ "obj-7", 4 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-225", 1 ],
                    "midpoints": [ 1893.1189010858536, 390.0, 1971.5, 390.0 ],
                    "source": [ "obj-7", 3 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-225", 0 ],
                    "midpoints": [ 1808.2459340572357, 390.0, 1730.5, 390.0 ],
                    "source": [ "obj-7", 2 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-131", 0 ],
                    "midpoints": [ 680.508363366127, 162.0, 680.508363366127, 162.0 ],
                    "source": [ "obj-70", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-258", 0 ],
                    "midpoints": [ 3124.5, 99.0, 3124.5, 99.0 ],
                    "source": [ "obj-71", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-54", 0 ],
                    "midpoints": [ 478.5, 600.0, 478.5, 600.0 ],
                    "source": [ "obj-73", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-95", 0 ],
                    "midpoints": [ 478.5, 654.0, 478.5, 654.0 ],
                    "source": [ "obj-75", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-143", 0 ],
                    "midpoints": [ 2833.5, 240.0, 2685.5, 240.0 ],
                    "order": 1,
                    "source": [ "obj-8", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-145", 0 ],
                    "midpoints": [ 2833.5, 216.0, 2833.5, 216.0 ],
                    "order": 0,
                    "source": [ "obj-8", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-70", 0 ],
                    "midpoints": [ 680.508363366127, 117.0, 680.508363366127, 117.0 ],
                    "source": [ "obj-80", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-3", 0 ],
                    "midpoints": [ 2130.5, 306.0, 2130.5, 306.0 ],
                    "source": [ "obj-83", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-94", 0 ],
                    "midpoints": [ 2330.5, 231.0, 2330.5, 231.0 ],
                    "source": [ "obj-84", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-94", 0 ],
                    "midpoints": [ 2488.5, 267.0, 2330.5, 267.0 ],
                    "source": [ "obj-85", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-128", 0 ],
                    "midpoints": [ 2246.8333333333335, 171.0, 2246.5, 171.0 ],
                    "order": 1,
                    "source": [ "obj-86", 2 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-26", 0 ],
                    "midpoints": [ 2225.5, 195.0, 2130.5, 195.0 ],
                    "source": [ "obj-86", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-83", 0 ],
                    "midpoints": [ 2257.5, 267.0, 2130.5, 267.0 ],
                    "source": [ "obj-86", 3 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-84", 0 ],
                    "midpoints": [ 2236.1666666666665, 195.0, 2330.5, 195.0 ],
                    "source": [ "obj-86", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-85", 0 ],
                    "midpoints": [ 2246.8333333333335, 195.0, 2488.5, 195.0 ],
                    "order": 0,
                    "source": [ "obj-86", 2 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-86", 0 ],
                    "midpoints": [ 2225.5, 120.0, 2225.5, 120.0 ],
                    "source": [ "obj-87", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-184", 0 ],
                    "midpoints": [ 931.5, 516.0, 931.5, 516.0 ],
                    "source": [ "obj-88", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-103", 0 ],
                    "midpoints": [ 2330.5, 468.0, 2330.5, 468.0 ],
                    "source": [ "obj-89", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-60", 0 ],
                    "midpoints": [ 478.5, 402.0, 478.5, 402.0 ],
                    "source": [ "obj-9", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-214", 0 ],
                    "midpoints": [ 2330.5, 402.0, 2330.5, 402.0 ],
                    "source": [ "obj-90", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-116", 0 ],
                    "midpoints": [ 200.5, 78.0, 200.5, 78.0 ],
                    "source": [ "obj-91", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-36", 0 ],
                    "midpoints": [ 3172.5, 285.0, 3124.5, 285.0 ],
                    "source": [ "obj-92", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-90", 1 ],
                    "midpoints": [ 2389.5, 363.0, 2341.0, 363.0 ],
                    "source": [ "obj-93", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-90", 0 ],
                    "midpoints": [ 2330.5, 354.0, 2330.5, 354.0 ],
                    "source": [ "obj-93", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-93", 0 ],
                    "midpoints": [ 2449.5, 315.0, 2330.5, 315.0 ],
                    "source": [ "obj-94", 1 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-120", 0 ],
                    "midpoints": [ 225.5, 207.0, 225.5, 207.0 ],
                    "order": 1,
                    "source": [ "obj-96", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-124", 0 ],
                    "midpoints": [ 225.5, 219.0, 266.5, 219.0 ],
                    "order": 0,
                    "source": [ "obj-96", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-97", 0 ],
                    "midpoints": [ 225.5, 219.0, 148.5, 219.0 ],
                    "order": 2,
                    "source": [ "obj-96", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-28", 0 ],
                    "midpoints": [ 148.5, 360.0, 148.5, 360.0 ],
                    "source": [ "obj-97", 0 ]
                }
            }
        ],
        "parameters": {
            "obj-41": [ "live.gain~", "master", 0 ],
            "parameterbanks": {
                "0": {
                    "index": 0,
                    "name": "",
                    "parameters": [ "-", "-", "-", "-", "-", "-", "-", "-" ],
                    "buttons": [ "-", "-", "-", "-", "-", "-", "-", "-" ]
                }
            },
            "inherited_shortname": 1
        },
        "autosave": 0,
        "styles": [
            {
                "name": "Audiomix",
                "default": {
                    "bgfillcolor": {
                        "angle": 270.0,
                        "color": [ 0.290196, 0.309804, 0.301961, 1.0 ],
                        "color1": [ 0.376471, 0.384314, 0.4, 1.0 ],
                        "color2": [ 0.290196, 0.309804, 0.301961, 1.0 ],
                        "proportion": 0.39,
                        "type": "gradient"
                    }
                },
                "parentstyle": "",
                "multi": 0
            }
        ]
    }
}