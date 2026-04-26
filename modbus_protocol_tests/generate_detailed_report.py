from __future__ import annotations

import argparse
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

if __package__ is None or __package__ == "":
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from modbus_protocol_tests.test_protocol_regression import TEST_COVERAGE_MAP, TEST_EXPECTATION_MAP


def _extract_case_base_name(case_name: str) -> str:
    if "[" in case_name:
        return case_name.split("[", 1)[0]
    return case_name


def _load_cases(xml_path: Path) -> list[dict[str, str]]:
    root = ET.parse(xml_path).getroot()
    cases: list[dict[str, str]] = []

    for testcase in root.iter("testcase"):
        classname = testcase.attrib.get("classname", "")
        name = testcase.attrib.get("name", "")
        duration = testcase.attrib.get("time", "0")

        status = "passed"
        detail = ""
        if testcase.find("failure") is not None:
            status = "failed"
            failure = testcase.find("failure")
            detail = (failure.text or failure.attrib.get("message", "")).strip()
        elif testcase.find("error") is not None:
            status = "error"
            error = testcase.find("error")
            detail = (error.text or error.attrib.get("message", "")).strip()
        elif testcase.find("skipped") is not None:
            status = "skipped"
            skipped = testcase.find("skipped")
            detail = (skipped.attrib.get("message", "") or (skipped.text or "")).strip()

        base_name = _extract_case_base_name(name)
        coverage_items = TEST_COVERAGE_MAP.get(base_name, ["(未登记覆盖点)"])
        expected_type = TEST_EXPECTATION_MAP.get(base_name, "(未登记预期类型)")

        cases.append(
            {
                "node": f"{classname}::{name}" if classname else name,
                "case": name,
                "status": status,
                "duration_seconds": duration,
                "expected_result_type": expected_type,
                "coverage": "；".join(coverage_items),
                "assertion_output": detail,
            }
        )

    return cases


def _render_markdown(cases: list[dict[str, str]]) -> str:
    total = len(cases)
    passed = sum(1 for c in cases if c["status"] == "passed")
    failed = sum(1 for c in cases if c["status"] == "failed")
    errored = sum(1 for c in cases if c["status"] == "error")
    skipped = sum(1 for c in cases if c["status"] == "skipped")

    lines = [
        "# Modbus 协议详细测试报告",
        "",
        "## 总览",
        "",
        f"- 总用例数: {total}",
        f"- 通过: {passed}",
        f"- 失败: {failed}",
        f"- 错误: {errored}",
        f"- 跳过: {skipped}",
        "",
        "## 用例明细",
        "",
        "| 用例 | 预期结果类型 | 覆盖内容 | 状态 | 耗时(s) | 错误/断言输出 |",
        "|---|---|---|---|---:|---|",
    ]

    for case in cases:
        assertion = case["assertion_output"].replace("\n", "<br>").replace("|", "\\|")
        if not assertion:
            assertion = "-"

        status = case["status"]
        if status == "passed":
            status_text = "PASS"
        elif status == "failed":
            status_text = "FAIL"
        elif status == "error":
            status_text = "ERROR"
        else:
            status_text = "SKIP"

        lines.append(
            f"| {case['case']} | {case['expected_result_type']} | {case['coverage']} | {status_text} | {case['duration_seconds']} | {assertion} |"
        )

    return "\n".join(lines) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(description="根据 pytest junit xml 生成详细报告")
    parser.add_argument("--junit-xml", required=True)
    parser.add_argument("--markdown-out", required=True)
    parser.add_argument("--json-out", required=True)
    args = parser.parse_args()

    xml_path = Path(args.junit_xml)
    md_path = Path(args.markdown_out)
    json_path = Path(args.json_out)

    cases = _load_cases(xml_path)

    md_path.write_text(_render_markdown(cases), encoding="utf-8")
    json_path.write_text(json.dumps({"cases": cases}, ensure_ascii=False, indent=2), encoding="utf-8")

    print(f"详细 Markdown 报告已写入: {md_path}")
    print(f"详细 JSON 报告已写入: {json_path}")


if __name__ == "__main__":
    main()
