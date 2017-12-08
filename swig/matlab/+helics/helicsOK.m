function v = helicsOK()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 0);
  end
  v = vInitialized;
end
