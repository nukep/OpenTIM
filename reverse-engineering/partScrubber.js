
const OFF_X = 8
const OFF_Y = 47

function shallowCloneMap(o) {
    const newObj = {}
    for (const k in o) {
        newObj[k] = { _tmp: o[k] }
    }
    return newObj
}

function normalizeData(data) {
    data = {
        parts: shallowCloneMap(data.parts),
        belts: shallowCloneMap(data.belts),
        ropes: shallowCloneMap(data.ropes),
    }

    const norm = (obj) => {
        for (const fieldName in obj._tmp) {
            const field = obj._tmp[fieldName]
            let val = field.value
            if (field.type == 'Part') {
                val = data.parts[field.repr_value]
            } else if (field.type == 'PartType') {
                val = field.repr_value
            } else if (field.type == 'BeltData') {
                val = data.belts[field.repr_value]
            } else if (field.type == 'RopeData') {
                val = data.ropes[field.repr_value]
            }
            obj[fieldName] = val
        }
        delete obj._tmp
    }

    for (const addr in data.parts) {
        norm(data.parts[addr])
    }
    for (const addr in data.belts) {
        norm(data.belts[addr])
    }
    for (const addr in data.ropes) {
        norm(data.ropes[addr])
    }
    return data
}

function calculateRopeSag(part, rope) {
    let nextpart
    if (part.type == 'PULLEY') {
        nextpart = part.links_to_1
    } else {
        nextpart = rope.part1_rope_slot == 0 ? part.links_to_1 : part.links_to_2
    }
    const rope_part = rope.rope_or_pulley_part

    if (rope.part1 == part) {
        return rope_part.extra1 - approximateHypotOfRope(rope, 0)
    } else {
        if (nextpart == null){
            return 0
        }
        if (rope.part2 != nextpart) {
            return 0
        }

        return rope_part.extra2 - approximateHypotOfRope(rope, 1)
    }
}

function approximateHypotOfRope(rope, first_or_last) {
    const f = (r, field, i) => {
        const name = field + ((i == 0) ? '_1' : '_2')
        return r[name]
    }

    let l
    if (first_or_last == 0) {
        const v = f(rope.part1, 'links_to', rope.part1_rope_slot)

        if (rope.part2 == v) {
            l = [[rope, 0], [rope, 1]]
        } else {
            l = [[rope, 0], [v.rope_data_1, 0]]
        }
    } else {
        if (rope.part2 == null) {
            return 0
        }

        const v = f(rope.part2, 'links_to', rope.part2_rope_slot)
        if (rope.part1 == v) {
            l = [[rope, 1], [rope, 0]]
        } else {
            l = [[rope, 1], [v.rope_data_1, 1]]
        }
    }

    const [[a, i_a], [b, i_b]] = l

    if (b == null) {
        return 0
    }

    const dx = Math.abs(f(a, 'ends_pos', i_a)[0] - f(b, 'ends_pos', i_b)[0])
    const dy = Math.abs(f(a, 'ends_pos', i_a)[1] - f(b, 'ends_pos', i_b)[1])

    if (dx < dy) {
        return Math.floor(dx/4) + Math.floor(dx/8) + dy
    } else {
        return Math.floor(dy/4) + Math.floor(dy/8) + dx
    }
}

function App() {
    const [frame, setFrame] = React.useState(1)
    const sliderChange = (e) => {
        setFrame(parseInt(e.target.value))
    }
    const [focusedObject, setFocusedObject] = React.useState(null)

    const [ALLDATA, setALLDATA] = React.useState(null)
    const frameCount = ALLDATA && ALLDATA.length
    const currentData = ALLDATA && ALLDATA[frame-1]
    
    React.useEffect(() => {
        fetch('parts.json').then(r => r.json()).then(value => {
            setALLDATA(value)
        })
    }, [])

    const imgPath = `Scenario1/screen${frame.toString(16).toUpperCase()}.png`

    let boxes = []
    let ropeLines = []

    if (currentData) {
        const { parts, belts, ropes } = normalizeData(currentData)
        for (const addr in parts) {
            const part = parts[addr]
            
            const [x, y] = part.pos_render
            const [ox, oy] = part.pos
            const [w, h] = part.size

            if (w > 0 && h > 0) {
                boxes.push([addr, x, y, w, h])
            }

        }
        for (const addr in parts) {
            const part = parts[addr]
            if (part.type == 'ROPE') {
                const rope = part.rope_data_1
                let curpart = rope.part1
                let nextpart = rope.part1_rope_slot == 0 ? curpart.links_to_1 : curpart.links_to_2
                if (nextpart == null) {
                    nextpart = rope.part2
                }

                while (curpart && nextpart) {
                    let x1, y1, x2, y2, x3, y3
                    if (curpart.type == 'PULLEY') {
                        const rpd = curpart.rope_data_1
                        x1 = rpd.ends_pos_2[0]
                        y1 = rpd.ends_pos_2[1]
                    } else {
                        x1 = rope.ends_pos_1[0]
                        y1 = rope.ends_pos_1[1]
                    }
                    if (nextpart.type == 'PULLEY') {
                        const rpd = nextpart.rope_data_1
                        x3 = rpd.ends_pos_1[0]
                        y3 = rpd.ends_pos_1[1]
                    } else {
                        x3 = rope.ends_pos_2[0]
                        y3 = rope.ends_pos_2[1]
                    }

                    x2 = (x1 + x3)/2
                    y2 = (y1 + y3)/2

                    if (!(curpart.type == 'PULLEY' && nextpart.type == 'PULLEY')) {
                        const sag = calculateRopeSag(curpart, rope)
                        y2 += sag
                    }

                    ropeLines.push([x1, y1, x2, y2, x3, y3])

                    if (nextpart.type != 'PULLEY') {
                        break
                    }

                    curpart = nextpart
                    nextpart = nextpart.links_to_1
                }
            }
        }
    }

    const onFocusObject = (addr, type) => () => {
        setFocusedObject([addr, type])
    }
    const onNoObject = () => {
        setFocusedObject(null)
    }

    let focusedObjectFields = null
    
    if (currentData && focusedObject) {
        const [addr, type] = focusedObject
        const s = ({
            'part': 'parts',
            'belt': 'belts',
            'rope': 'ropes'
        })[type]

        focusedObjectFields = currentData[s][addr]
    }

    return <div>
        {frameCount && <div>
            <input type="range" min="1" max={frameCount} value={frame} onChange={sliderChange} />
        </div>}
        <svg width="640" height="480">
            {/* <image href={`./Scenario1/screen${frame}.png`}  onClick={onNoObject} /> */}
            <g transform={`translate(${OFF_X} ${OFF_Y})`}>
            {ropeLines.map(([x1, y1, x2, y2, x3, y3]) => {
                return <path d={`M${x1},${y1} Q${x2},${y2} ${x3},${y3}`} stroke="black" fill="none" stroke-width="5" />
            })}
            {boxes.map(([addr, x, y, w, h]) => {
                return <rect key={addr} x={x} y={y} width={w} height={h} fill="rgba(0,0,0,0.5)" onClick={onFocusObject(addr, 'part')} />
            })}
            </g>
        </svg>
        {focusedObjectFields && <div>
            <div>{JSON.stringify(focusedObject)}</div>
            <table>
                <thead>
                <tr>
                    <th>Name</th>
                    <th>Value</th>
                </tr>
                </thead>
                <tbody>
                    {Object.entries(focusedObjectFields).map(([k, v]) => {
                        let linkToObj = null
                        if (v.value !==  0) {
                            if (v.type == 'Part') {
                                linkToObj = [v.repr_value, 'part']
                            }
                            if (v.type == 'BeltData') {
                                linkToObj = [v.repr_value, 'belt']
                            }
                            if (v.type == 'RopeData') {
                                linkToObj = [v.repr_value, 'rope']
                            }
                        }
                        if (linkToObj) {
                            return <tr>
                                <td>{k}</td>
                                <td><a href="#" onClick={() => setFocusedObject(linkToObj)}>{JSON.stringify(v.repr_value)}</a></td>
                            </tr>
                        } else {
                            return <tr>
                                <td>{k}</td>
                                <td>{JSON.stringify(v.repr_value)}</td>
                            </tr>
                        }
                    })}
                </tbody>
            </table>
        </div>}
        <div>{focusedObject}</div>
    </div>
}

ReactDOM.render(
    <App />,
    document.getElementById('root')
);