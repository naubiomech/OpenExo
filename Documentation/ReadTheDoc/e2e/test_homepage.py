import re

from playwright.sync_api import Page, expect

from conftest import BASE_URL


def test_homepage(page: Page):
    page.goto(BASE_URL)
    # Expect a title "to contain" a substring.
    expect(page).to_have_title(re.compile("Piccolo Theme"))

    # Open sidebar
    page.locator('#toggle_sidebar').click()
    expect(page.locator('.sphinxsidebar')
           ).to_have_attribute("style", "display: none;")

    # Dark Mode
    page.goto(BASE_URL)
    expect(page.locator('html')).to_have_attribute("data-mode", "light")
    page.locator("#mode_toggle").click()
    expect(page.locator('html')).to_have_attribute("data-mode", "dark")
    page.locator("#mode_toggle").click()
    expect(page.locator('html')).to_have_attribute("data-mode", "darkest")

    # Search for Configuration
    page.goto(BASE_URL)
    page.locator(
        '//*[@id="searchbox"]/div/form/input[1]').fill("Configuration")
    page.locator('//*[@id="searchbox"]/div/form/input[2]').click()
    expect(page).to_have_title(re.compile("Search"))

    # Check external Sphinx Link
    page.goto(BASE_URL)
    link = page.locator('//*[@id="piccolo-theme"]/p/a')
    expect(link).to_have_text("Sphinx")
    expect(link).to_have_attribute(
        "href", "https://www.sphinx-doc.org/en/master/")

    # Link Check Setup
    page.goto(BASE_URL)
    page.locator('//*[@id="piccolo-theme"]/div/ul/li[1]/a').click()
    expect(page).to_have_title(re.compile("Setup"))

    # Github Link
    page.locator('#source_link').click()
    expect(page).to_have_url(re.compile(
        'https://github.com/piccolo-orm/piccolo_theme/'))
