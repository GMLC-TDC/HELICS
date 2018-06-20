function v = helics_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176312);
  end
  v = vInitialized;
end
