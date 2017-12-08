function v = error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 8);
  end
  v = vInitialized;
end
