function v = helics_startup_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176320);
  end
  v = vInitialized;
end
